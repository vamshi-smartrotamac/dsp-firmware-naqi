/**************************************************************************
 * Copyright (c) 2025 Naqi Logix Inc. All Rights Reserved.
 *
 * File: Communication.h
 *
 * This file is part of the proprietary firmware and is protected by
 * applicable copyright laws and international treaties. Unauthorized use,
 * reproduction, or distribution of this file or its contents is strictly
 * prohibited. For permissions, contact info @ naqilogix.com.
 **************************************************************************/

#ifndef COMMUNICATION_H_
#define COMMUNICATION_H_

#include <stdint.h>
#include <uart.h>

/* Private macros -----------------------------------------------------------*/
#define COMMUNICATION_TASK_NAME "Communication"

#define UART0_BAUD_RATE         1000000  // 1Mbps
#define UART0                   MXC_UART_GET_UART(0)

#define Rx_PACKET_SIZE_MAX      9

#define UART_QUEUE_DEPTH        4
#define COM_TX_PACKET_MAX_SIZE  69

typedef struct
{
    uint8_t data[COM_TX_PACKET_MAX_SIZE];
    uint8_t len;
} ComTxMessage_t;

int    UART_Initialization(mxc_uart_regs_t *uart);
int8_t Com_initialization(void);
int    Com_transmit(uint8_t *txBuf, uint32_t len);
int    Com_AddToQueue(ComTxMessage_t *msg);
void   Com_task(void *pvParameters);

#endif /* COMMUNICATION_H_ */
