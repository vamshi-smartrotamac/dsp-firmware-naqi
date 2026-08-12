/**************************************************************************
 * Copyright (c) 2025 Naqi Logix Inc. All Rights Reserved.
 *
 * File: FirmwareUpgrade.c
 *
 * This file is part of the proprietary firmware and is protected by
 * applicable copyright laws and international treaties. Unauthorized use,
 * reproduction, or distribution of this file or its contents is strictly
 * prohibited. For permissions, contact info @ naqilogix.com.
 **************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "mxc_device.h"
#include "uart.h"
#include "dma.h"
#include "flc.h"
#include "nvic_table.h"
#include "logger.h"

#include "FirmwareUpgrade.h"

/* Private macros -----------------------------------------------------------*/
#define FLASH_ERASE_PAGE_ADDR   (0x1007E000)    /* Last page of flash for upgrade command flag */
#define FUPGRADE_CMD             "fupgrade\r"
#define FUPGRADE_CMD_LEN         (sizeof(FUPGRADE_CMD) - 1)   /* exclude the C-string NUL terminator */

bool Dfu_isRequested(const uint8_t *buf, size_t len)
{
    return (len >= FUPGRADE_CMD_LEN) && !memcmp(buf, FUPGRADE_CMD, FUPGRADE_CMD_LEN);
}

void Dfu_trigger(void)
{
    // Stop peripherals (IMPORTANT)
    MXC_UART_Shutdown(MXC_UART_GET_UART(0));

    // Stop DMA explicitly if used
    MXC_DMA_Stop(0);   // DMA0
    MXC_DMA_Stop(1);   // DMA1

    // Suspend scheduler
    vTaskSuspendAll();

    // Disable interrupts globally
    __disable_irq();

    // Ensure no flash execution conflict
    __DSB();
    __ISB();

    MXC_FLC_PageErase(FLASH_ERASE_PAGE_ADDR);

    NVIC_SystemReset();
}
