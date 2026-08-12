/******************************************************************************
 *
 * Copyright (C) 2023-2024 Analog Devices, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************************/

/**
 * @file        FreeRTOS_Debug.c
 * @brief       FreeRTOS Debug utilities including RTOS Stats Timer
 *              and template HardFault Handler
 */
#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include "tmr.h"
#include "nvic_table.h"
#include "mxc.h"
#include "logger.h"

/* ===========================================================================
 * RTOS run-time stats timer
 * ---------------------------------------------------------------------------
 * IMPORTANT: These two functions are referenced by the FreeRTOS kernel
 * (tasks.c -> vTaskStartScheduler / vTaskSwitchContext) whenever the kernel is
 * compiled with configGENERATE_RUN_TIME_STATS == 1.
 *
 * They are defined UNCONDITIONALLY here (i.e. NOT behind #if DEBUG), on
 * purpose. The FreeRTOS library (librtos.a) and this application are separate
 * compilation units; if they are ever built with a different view of DEBUG --
 * for example a Release app linking against a librtos.a that was built (or left
 * stale) with DEBUG=1 -- the kernel will reference these symbols while a
 * DEBUG-gated definition would be missing, producing:
 *     undefined reference to `ConfigTimerForStats'
 *     undefined reference to `GetTimerForStats'
 *
 * Defining them unconditionally guarantees the symbols always resolve. When
 * run-time stats are disabled, nothing calls them and the linker's
 * --gc-sections drops them, so there is no image-size cost in Release.
 *
 * Scheduler tick is 1ms; RTOS stats clock should be 10-100x faster.
 *
 * NOTE: This timer rolls over after 2^32 / (TMR_SRC / prescaler) seconds.
 * Because the RTOS tracks total time, rollover cannot be protected by the
 * application and will affect statistics over long measurement windows.
 * EXAMPLE: APB clock source with 2048 prescale -> 2^32 / 32000 = ~93 days.
 * ===========================================================================*/

#define RTOS_STATS_TMR MXC_TMR0
#define RTOS_STATS_TMR_SRC MXC_TMR_32K_CLK
#define RTOS_STATS_TMR_CNT 0xFFFFFFFF

void ConfigTimerForStats(void)
{
    mxc_tmr_cfg_t tmr;

    MXC_TMR_Shutdown(RTOS_STATS_TMR);

    tmr.pres = MXC_TMR_PRES_1;
    tmr.mode = TMR_MODE_CONTINUOUS;
    tmr.bitMode = MXC_TMR_BIT_MODE_32;
    tmr.clock = RTOS_STATS_TMR_SRC;
    tmr.cmp_cnt = RTOS_STATS_TMR_CNT; // SystemCoreClock*(1/interval_time);
    tmr.pol = 0;
    MXC_TMR_Init(RTOS_STATS_TMR, &tmr, true);

    MXC_TMR_Start(RTOS_STATS_TMR);
}

uint32_t GetTimerForStats(void)
{
    return MXC_TMR_GetCount(RTOS_STATS_TMR);
}

/* ===========================================================================
 * Debug-only fault handling
 * Kept behind DEBUG so Release uses the CMSIS weak default handlers. Move the
 * block outside the guard if you want fault capture in Release builds too.
 * ===========================================================================*/
#if defined(DEBUG) && (DEBUG == 1)

void prvGetRegistersFromStack(uint32_t *pulFaultStackAddress)
{
    /* These are volatile to try and prevent the compiler/linker optimising them
    away as the variables never actually get used.  If the debugger won't show the
    values of the variables, make them global my moving their declaration outside
    of this function. */
    volatile uint32_t r0;
    volatile uint32_t r1;
    volatile uint32_t r2;
    volatile uint32_t r3;
    volatile uint32_t r12;
    volatile uint32_t lr; /* Link register. */
    volatile uint32_t pc; /* Program counter. */
    volatile uint32_t cfsr; /* Configurable Fault Status Register (MemManage/Bus/Usage). */
    volatile uint32_t psr; /* Program status register. */
    volatile uint32_t hfsr;  /* HardFault Status Register. */
    volatile uint32_t mmfar; /* MemManage Fault Address Register. */
    volatile uint32_t bfar;  /* BusFault Address Register. */

    r0 = pulFaultStackAddress[0];
    r1 = pulFaultStackAddress[1];
    r2 = pulFaultStackAddress[2];
    r3 = pulFaultStackAddress[3];

    r12 = pulFaultStackAddress[4];
    lr = pulFaultStackAddress[5];
    pc = pulFaultStackAddress[6];
    psr = pulFaultStackAddress[7];

    cfsr = SCB->CFSR;
    hfsr = SCB->HFSR;
    mmfar = SCB->MMFAR;
    bfar = SCB->BFAR;
    NAQILOG_PRINTF("\r\n\r\n*** HARD FAULT ***\r\n");
    NAQILOG_PRINTF("R0    = 0x%08lX\r\n", (unsigned long)r0);
    NAQILOG_PRINTF("R1    = 0x%08lX\r\n", (unsigned long)r1);
    NAQILOG_PRINTF("R2    = 0x%08lX\r\n", (unsigned long)r2);
    NAQILOG_PRINTF("R3    = 0x%08lX\r\n", (unsigned long)r3);
    NAQILOG_PRINTF("R12   = 0x%08lX\r\n", (unsigned long)r12);
    NAQILOG_PRINTF("LR    = 0x%08lX\r\n", (unsigned long)lr);
    NAQILOG_PRINTF("PC    = 0x%08lX\r\n", (unsigned long)pc);
    NAQILOG_PRINTF("PSR   = 0x%08lX\r\n", (unsigned long)psr);
    NAQILOG_PRINTF("CFSR  = 0x%08lX\r\n", (unsigned long)cfsr);
    NAQILOG_PRINTF("HFSR  = 0x%08lX\r\n", (unsigned long)hfsr);
    NAQILOG_PRINTF("MMFAR = 0x%08lX\r\n", (unsigned long)mmfar);
    NAQILOG_PRINTF("BFAR  = 0x%08lX\r\n", (unsigned long)bfar);
    NAQILOG_PRINTF("*** halted ***\r\n");
    fflush(stdout);
    /* When the following line is hit, the variables contain the register values. */
    for (;;)
    {
    }
}

/* The prototype shows it is a naked function - in effect this is just an
assembly function. */
void HardFault_Handler(void) __attribute__((naked, aligned(8)));

/* The fault handler implementation calls a function called
prvGetRegistersFromStack(). */
void HardFault_Handler(void)
{
    __asm volatile(" tst lr, #4                                                \n"
                   " ite eq                                                    \n"
                   " mrseq r0, msp                                             \n"
                   " mrsne r0, psp                                             \n"
                   " ldr r1, [r0, #24]                                         \n"
                   " ldr r2, handler2_address_const                            \n"
                   " bx r2                                                     \n"
                   " handler2_address_const: .word prvGetRegistersFromStack    \n");
}
#endif

/* Stack-overflow hook. configCHECK_FOR_STACK_OVERFLOW is enabled unconditionally
 * in FreeRTOSConfig.h, so the FreeRTOS kernel references this symbol in EVERY
 * build (debug AND release). It must therefore be defined outside the DEBUG
 * guard above. The diagnostic print uses the debug-only NAQILOG_PRINTF macro,
 * so that call is guarded internally; in release the hook simply halts. */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    (void)pcTaskName;
    taskDISABLE_INTERRUPTS();

#if defined(DEBUG) && (DEBUG == 1)
    NAQILOG_PRINTF("\r\n\r\n*** STACK OVERFLOW in task '%s' ***\r\n", (const char *)pcTaskName);
    fflush(stdout);
#endif

    for (;;)
    {
    }
}
