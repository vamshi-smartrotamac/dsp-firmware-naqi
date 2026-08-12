/**
 * @file SystemServices.c
 * @author pierre@wisear.io
 * @brief
 * @version 0.1
 * @date 2024-02-06
 *
 * @copyright Copyright (c) 2024
 *
 */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"

#include "ExGacquisition.h"
#include "Communication.h"
#include "IMUacquisition.h"
#include "SystemServices.h"
#include "logger.h"
#include "i2c_driver.h"
#include "GestureProcessing.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
#define COMMUNICATION_TASK_PRIO 2
#define IMU_ACQUISITION_TASK_PRIO 3
#define EXG_ACQUISITION_TASK_PRIO 3
#define PROCESSING_TASK_PRIO 4

#define SYS_STATE_BUFFER_SIZE 0x100

/* Global variables ----------------------------------------------------------*/
static TaskHandle_t imuAcquisitionHandle = NULL;
static TaskHandle_t exgAcquisitionHandle = NULL;
static TaskHandle_t processingHandle = NULL;
static TaskHandle_t communicationHandle = NULL;

/* Private function prototypes -----------------------------------------------*/

/**
 * @brief Start application. Create RTOS resources and start tasks. Configure AFE.
 *
 */
void Sys_start(void)
{
    // NVIC priority grouping configuration: no subpriorities, only preemptive priorities (0 to 7)
    // This is required for FreeRTOS API calls from interrupts with priority 5 or higher (numerically lower)
    NVIC_SetPriorityGrouping(0);

    // Communication initialization
    int8_t comInitStatus = Com_initialization();
    if (comInitStatus != 0)
    {
        NAQILOG_ERROR("Communication initialization failed!\r");
    }

    // AFE acquisition initialization
    int8_t exgInitStatus = Exg_initializeAcquisitionAfe(JC_PROCESSING_SIZE_WINDOW_INPUT);
    if (exgInitStatus != 0)
    {
        NAQILOG_ERROR("ExG acquisition initialization failed!\r");
    }

    // IMU acquisition initialization
    int8_t imuInitStatus = Imu_initialization(MXC_I2C1);
    if (imuInitStatus != 0)
    {
        NAQILOG_ERROR("IMU acquisition initialization failed!\r");
    }

    // Processing initialization
    int8_t processingInitStatus = Gproc_initProcessing();
    if (processingInitStatus < 0)
    {
        NAQILOG_ERROR("Processing initialization failed!\r");
    }

    /* Tasks creation */
    if (comInitStatus == 0)
    {
        if (xTaskCreate(Com_task, COMMUNICATION_TASK_NAME, 0x200, NULL,
                        tskIDLE_PRIORITY + COMMUNICATION_TASK_PRIO, &communicationHandle) != pdPASS)
        {
            NAQILOG_ERROR("UART Communication task creation failed!\r");
        }
    }

    if (exgInitStatus == 0)
    {
        if (xTaskCreate(Exg_acquireDataTask, ACQUISITION_TASK_NAME_EXG, 0x200, NULL,
                        tskIDLE_PRIORITY + EXG_ACQUISITION_TASK_PRIO, &exgAcquisitionHandle) != pdPASS)
        {
            NAQILOG_ERROR("EXG Acquisition task creation failed!\r");
        }
    }

    if (imuInitStatus == 0)
    {
        if (xTaskCreate(Imu_acquireDataTask, ACQUISITION_TASK_NAME_IMU, 0x400, NULL,
                        tskIDLE_PRIORITY + IMU_ACQUISITION_TASK_PRIO, &imuAcquisitionHandle) != pdPASS)
        {
            NAQILOG_ERROR("IMU acquisition task creation failed!\r");
        }
    }

    if (processingInitStatus == 0 && exgInitStatus == 0)
    {
        if (xTaskCreate(Gproc_runProcessingTask, PROCESSING_TASK_NAME, 0x800, NULL,
                        tskIDLE_PRIORITY + PROCESSING_TASK_PRIO, &processingHandle) != pdPASS)
        {
            NAQILOG_ERROR("Processing task creation failed!\r");
        }
    }

    NAQILOG_INFO("Start application\r");
    vTaskStartScheduler();
}

#ifdef DEBUG
/**
 * @brief Print tasks statistics
 *
 */
static void reportSystemState(void)
{
    static char systemStateBuffer[SYS_STATE_BUFFER_SIZE];

    // One task requires ~40 bytes
    // Timer service and idle task are included too
    // For more info, see:
    // https://www.freertos.org/Documentation/02-Kernel/04-API-references/03-Task-utilities/00-Task-utilities#vtasklist
    if (uxTaskGetNumberOfTasks() * 40 > SYS_STATE_BUFFER_SIZE)
    {
        NAQILOG_WARN("System state buffer size too small (task count: %ld): could not print system state report",
                     uxTaskGetNumberOfTasks());
    }
    else
    {
        vTaskList(systemStateBuffer);
        NAQILOG_INFO("System state:\n"
                     "Name          State   Priority  Stack  Num \n"
                     "*******************************************\n"
                     "%s",
                     systemStateBuffer);
    }
}
#endif /* DEBUG */
