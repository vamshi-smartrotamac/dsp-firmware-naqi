/**
 * @file DataAcquisition.c
 * @author pierre@wisear.io
 * @brief
 * @version 0.1
 * @date 2023-12-20
 *
 * @copyright Copyright (c) 2024
 *
 */

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include <string.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

#include "ComInterface.h"
#include "Communication.h"
#include "ExGacquisition.h"
#include "ExGacquisitionInterface.h"
#include "logger.h"
#include "mxc_delay.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Global variables ----------------------------------------------------------*/
static TaskHandle_t      acquisitionTaskHandle = NULL;
static SemaphoreHandle_t readWriteMutexHandle = NULL;
static uint16_t          bufferSize = 0;
static uint16_t          currentIndex = 0;
static int32_t          *exgCircularBuffer = NULL;

/**
 * @brief Initialize AFE Acquisition
 *
 * @param acquisitionBufferSizeAFE Size of the AFE acquisition buffer
 * @return Error code
 * @retval '0' - Success
 * @retval '-1' - Error
 */
int8_t Exg_initializeAcquisitionAfe(uint16_t acquisitionBufferSizeAFE)
{
    int8_t retCode = -1;

    if (exgCircularBuffer != NULL)
    {
        NAQILOG_WARN("AFE Acquisition already initialized\r");
    }
    // Acquisition resources initialization
    else if ((exgCircularBuffer = pvPortMalloc(acquisitionBufferSizeAFE * sizeof(int32_t))) != NULL)
    {
        memset(exgCircularBuffer, 0, acquisitionBufferSizeAFE * sizeof(int32_t));
        bufferSize = acquisitionBufferSizeAFE;
        readWriteMutexHandle = xSemaphoreCreateMutex();

        retCode = AfeItf_configure();
    }

    return retCode;
}

/**
 * @brief Function implementing the data acquisition thread.
 *
 * @param pvParameters Not used
 * @retval None
 */
void Exg_acquireDataTask(void *pvParameters)
{
    (void)pvParameters;

    TickType_t   xLastWakeTime;
    int32_t      sample;
    ExGSamples_t txMessage;
    uint16_t     exgSampleCount = 0;
    int8_t       ret;

    if (!exgCircularBuffer)
    {
        NAQILOG_ERROR("AFE acquisition not initialized!\r");
        // Stop execution
        return;
    }
    NAQILOG_INFO("AFE acquisition Task Launched!\r");

    acquisitionTaskHandle = xTaskGetCurrentTaskHandle();

    /* Zero out the message buffer */
    memset(&txMessage, 0, sizeof(ExGSamples_t));

    AfeItf_startAcquisition();

    /* Get task start time */
    xLastWakeTime = xTaskGetTickCount();

    for (;;)
    {
        ret = 0;

        // Lock AFE data buffer access
        if (xSemaphoreTake(readWriteMutexHandle, portMAX_DELAY) == pdTRUE)
        {
            if ((ret = AfeItf_readSample(&sample)) == 1)
            {
                exgCircularBuffer[currentIndex] = sample;
                currentIndex = (currentIndex + 1) % bufferSize;
            }
            // Unlock AFE data buffer access
            xSemaphoreGive(readWriteMutexHandle);
        }

        // The mechanism with ret is necessary in order to keep the semaphore guarded section as small as possible
        if (ret == 1)
        {
            txMessage.data[exgSampleCount] = sample;
            exgSampleCount++;

            if (exgSampleCount >= SAMPLES_PER_MESSAGE)
            {
                // Reset sample count for next message
                exgSampleCount = 0;

                AfeItf_getLeadOffData(&txMessage.leadOffI, &txMessage.leadOffQ, &txMessage.leadOffStatus);

                if (ComItf_sendExgData(&txMessage) != E_SUCCESS)
                {
                    NAQILOG_WARN("EXG packet transmission failed\r");
                }
            }
        }
        else if (ret == -1)
        {
            NAQILOG_ERROR("Failed reading samples from AFE");
        }

        // Read more often (333Hz) than the AFE generates samples because we cannot set exactly 256Hz
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(3));
    }
}

/**
 * @brief Function allowing to access AFE data samples acquired by the
 * acquisition task
 *
 * @param ptrDataBuffer Pointer to array of destination
 * @param dataBufferSize Size of the destination array
 * @param sampleCountToCopy Number of samples to copy into the array
 * @return Error code
 * @retval '0' - Success
 * @retval '-1' - Error
 */
int8_t Exg_copyFloatAFESamples(float *ptrDataBuffer, uint16_t dataBufferSize, uint16_t sampleCountToCopy)
{
    int8_t retCode = -1;

    if (exgCircularBuffer != NULL)
    {
        if (dataBufferSize >= sampleCountToCopy && bufferSize >= sampleCountToCopy)
        {
            if (xSemaphoreTake(readWriteMutexHandle, portMAX_DELAY) == pdTRUE)
            {
                for (uint16_t i = 0; i < sampleCountToCopy; i++)
                {
                    // The size of the array is added to the left operand of the modulus
                    // operation to prevent negative indexes
                    ptrDataBuffer[i] =
                        exgCircularBuffer[(i + currentIndex - sampleCountToCopy + bufferSize) % bufferSize];
                }

                xSemaphoreGive(readWriteMutexHandle);

                retCode = 0;
            }
        }
    }

    return retCode;
}
