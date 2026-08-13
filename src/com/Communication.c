/**************************************************************************
 * Copyright (c) 2025 Naqi Logix Inc. All Rights Reserved.
 *
 * File: Communication.c
 *
 * This file is part of the proprietary firmware and is protected by
 * applicable copyright laws and international treaties. Unauthorized use,
 * reproduction, or distribution of this file or its contents is strictly
 * prohibited. For permissions, contact info @ naqilogix.com.
 **************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "limits.h"
#include "queue.h"
#include "task.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "Communication.h"
#include "FirmwareUpgrade.h"
#include "led.h"
#include "logger.h"
#include "mxc.h"
#include "mxc_delay.h"
#include "nvic_table.h"
#include "uart.h"

/* Private macros -----------------------------------------------------------*/
#define NOTIFY_BIT_TX_DONE     (1UL << 0) /* set by txDMACallback         */
#define NOTIFY_BIT_DFU_REQUEST (1UL << 1) /* set by readCallback on "fupgrade\r" match */

/* Global variables ---------------------------------------------------------*/
static uint8_t rxDMABuf[Rx_PACKET_SIZE_MAX];  // DMA destination (internal)
static char    rxBuff[Rx_PACKET_SIZE_MAX];

static TaskHandle_t  xCommunicationTaskHandle = NULL;
static QueueHandle_t xUARTQueue = NULL;

static mxc_uart_req_t write_req;
static mxc_uart_req_t read_req;

/* Private function prototypes --------------------------------------------------*/

/**
 * @brief RX DMA completion callback - copies received data to rxBuff and checks for firmware
 *        upgrade command. Only notifies Com_task() on match; the actual flash erase and
 *        reset are deferred to task context since they block for the duration of the erase.
 * @param req    UART transaction descriptor (used for rxCnt)
 * @param error  Transaction status, E_NO_ERROR on success
 * @retval None
 */
static void readCallback(mxc_uart_req_t *req, int error)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (error == E_NO_ERROR)
    {
        memcpy((void *)rxBuff, rxDMABuf, req->rxCnt);

        if (Dfu_isRequested((const uint8_t *)rxBuff, req->rxCnt) && (xCommunicationTaskHandle != NULL))
        {
            xTaskNotifyFromISR(xCommunicationTaskHandle, NOTIFY_BIT_DFU_REQUEST, eSetBits, &xHigherPriorityTaskWoken);
        }

        // Clear RX buffer for next reception (this works if FW upgrade fails)
        memset((void *)rxBuff, 0, Rx_PACKET_SIZE_MAX);
    }
    // Re-arm DMA
    MXC_UART_TransactionDMA(&read_req);

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/**
 * @brief TX DMA completion callback - signals task notification to unblock Com_task()
 *
 * @param req    UART transaction descriptor (unused)
 * @param error  Transaction status (unused)
 * @retval None
 */
static void txDMACallback(mxc_uart_req_t *req, int error)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (xCommunicationTaskHandle != NULL)
    {
        /*
         * xTaskNotifyFromISR with eSetBits:
         *   - Sets NOTIFY_BIT_TX_DONE in the task's notification value.
         *   - Safe to call multiple times — bits accumulate, not overwritten.
         *   - Task clears the bit in xTaskNotifyWait().
         */
        xTaskNotifyFromISR(xCommunicationTaskHandle, NOTIFY_BIT_TX_DONE, eSetBits, &xHigherPriorityTaskWoken);
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/**
 * @brief  Fire TX DMA transmission.
 *
 * @param  txBuf  Pointer to data to transmit
 * @param  len    Number of bytes to send
 * @return E_NO_ERROR on success, error code on failure
 */
int Com_transmit(uint8_t *txBuf, uint32_t len)
{
    write_req.txData = txBuf;
    write_req.txLen = len;

    /* Fire TX DMA */
    return MXC_UART_TransactionDMA(&write_req);
}

/**
 * @brief  Enqueue a finished packet (EXG packet or IMU packet) for transmission.
 *         Non-blocking: if the queue is full, the message is dropped.
 *
 * @param  msg  Packet bytes and length to transmit
 * @return E_NO_ERROR on success, error code on failure
 */
int Com_AddToQueue(ComTxMessage_t *msg)
{
    if (xUARTQueue == NULL)
    {
        return E_UNINITIALIZED;
    }

    if (xQueueSend(xUARTQueue, msg, 0) == pdPASS)
    {
        return E_NO_ERROR;
    }
    else
    {
        NAQILOG_ERROR("UART queue full");
        return E_OVERFLOW;
    }
}

/**
 * @brief  Arms RX DMA for one PACKET_SIZE_MAX packet.
 *         Called once at init. readCallback() re-arms automatically.
 */
static void startRxDMA(void)
{
    read_req.uart = UART0;
    read_req.rxData = rxDMABuf;
    read_req.rxLen = Rx_PACKET_SIZE_MAX;
    read_req.txData = NULL;
    read_req.txLen = 0;
    read_req.callback = readCallback;

    MXC_UART_TransactionDMA(&read_req);
}

/*
 *---------------------------------------------------------------------------------------------
 * Function:      UART_Initialization
 * Description:   This function is Initialization of UART Peripheral
 *
 * Parameters:    [In1]mxc_uart_regs_t *uart              (UART Instance)
 *
 * return         Status of UART Initialization
 *  ---------------------------------------------------------------------------------------------
 */
int UART_Initialization(mxc_uart_regs_t *uart)
{
    int error = 0;

    if (!uart)
    {
        NAQILOG_ERROR("UART handle is NULL");
        return E_NULL_PTR;
    }

    /* Init UART peripheral once */
    error = MXC_UART_Init(uart, UART0_BAUD_RATE, MXC_UART_APB_CLK);
    if (error != E_NO_ERROR)
    {
        NAQILOG_ERROR("UART0 init failed: %d\r", error);
        return error;
    }

    /* Enable Auto DMA handlers for UART0 */
    MXC_UART_SetAutoDMAHandlers(uart, true);

    /* Init transmission */
    write_req.uart = uart;
    write_req.callback = txDMACallback;

    /* Set UART0, DMA0_IRQ, interrupt to safe priority for FreeRTOS API calls
     * Priority must be >= configMAX_SYSCALL_INTERRUPT_PRIORITY to safely call xTaskNotifyFromISR()
     * For Cortex-M4: typically priority 4 or 5 (numerically higher = safer for FreeRTOS)
     */
    NVIC_SetPriority(UART0_IRQn, 5);
    NVIC_SetPriority(DMA0_IRQn, 7);
    NVIC_SetPriority(DMA1_IRQn, 7);

    /*
     * Arm RX DMA immediately so we're ready to receive as soon as the first packet arrives from the QCC.
     * readCallback() re-arms DMA after each received packet.
     */
    startRxDMA();

    return E_NO_ERROR;
}

/**
 * @brief  Create the RTOS resources used by Com_task (TX queue).
 *
 * @return E_NO_ERROR on success, E_UNINITIALIZED on failure
 */
int8_t Com_initialization(void)
{
    xUARTQueue = xQueueCreate(UART_QUEUE_DEPTH, sizeof(ComTxMessage_t));
    if (xUARTQueue == NULL)
    {
        NAQILOG_ERROR("UART TX queue creation failed!\r");
        return E_UNINITIALIZED;
    }

    return E_NO_ERROR;
}

/**
 * @brief Function implementing the UART communication thread.
 *        Dequeues finished packets (EXG packet or IMU packet, built by
 *        ComInterface.c) and transmits them over UART, agnostic of content.
 *
 * @param pvParameters Not used
 * @retval None
 */
void Com_task(void *pvParameters)
{
    ComTxMessage_t txMsg;
    uint32_t       ulNotifiedValue;
    (void)pvParameters;

    NAQILOG_INFO("Comm task launched\r");

    /* Store current task handle for TX DMA callback notification */
    xCommunicationTaskHandle = xTaskGetCurrentTaskHandle();

    // mark TX as done so the first wait doesn't block
    xTaskNotify(xCommunicationTaskHandle, NOTIFY_BIT_TX_DONE, eSetBits);

    for (;;)
    {
        if (xTaskNotifyWait(pdFALSE,   /* Don't clear bits on entry. */
                            ULONG_MAX, /* Clear all bits on exit. */
                            &ulNotifiedValue, portMAX_DELAY) == pdPASS)
        {
            if (ulNotifiedValue & NOTIFY_BIT_DFU_REQUEST)
            {
                NAQILOG_INFO("Firmware upgrade command received, resetting\r");
                Dfu_trigger(); /* Blocks for the flash erase, then resets - never returns */
            }

            if ((xQueueReceive(xUARTQueue, &txMsg, portMAX_DELAY) == pdTRUE))
            {
                if (Com_transmit(txMsg.data, txMsg.len) != E_NO_ERROR)
                {
                    NAQILOG_ERROR("Packet transmission failed");
                    vTaskDelay(pdMS_TO_TICKS(10));
                    // Prevent com error from deadlocking the task
                    xTaskNotify(xCommunicationTaskHandle, NOTIFY_BIT_TX_DONE, eSetBits);
                }
            }
        }
    }
}
