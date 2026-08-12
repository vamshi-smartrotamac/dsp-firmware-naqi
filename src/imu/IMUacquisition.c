/**
 * @file IMUacquisition.c
 * @author pierre@wisear.io
 * @brief IMU acquisition task — woken by the BHI160 data-ready interrupt
 * @version 0.1
 * @date 2026-02-05
 *
 * @copyright Copyright (c) 2026
 *
 */
#include <limits.h>
#include <string.h>

#include "ComInterface.h"
#include "FreeRTOS.h"
#include "IMUacquisition.h"
#include "IMUacquisitionInterface.h"
#include "logger.h"
#include "task.h"

/* Private define ------------------------------------------------------------*/
#define IMU_DATA_READY_BIT (1UL << 0)

/* Private typedef -------------------------------------------------------------*/
typedef struct
{
    ImuSamples_t batch;
    uint8_t      accCount;
    uint8_t      gyrCount;
    uint8_t      quatCount;
    bool         magReady;
} ImuBatchAccumulator_t;

/* Global variables ----------------------------------------------------------*/
static TaskHandle_t IMUAcquisitionHandle = NULL;

/**
 * @brief   Folds one decoded FIFO sample into the batch being accumulated.
 * @param   acc    Accumulator state, updated in place.
 * @param   sample Decoded sample to fold in.
 * @return  true once acc/gyr/quat/mag are all present and @p acc->batch is
 *          ready to send.
 */
static bool accumulateSample(ImuBatchAccumulator_t *acc, ImuSampleEvent_t sample)
{
    switch (sample.type)
    {
        case IMU_SAMPLE_ACC:
            if (acc->accCount < IMU_SAMPLE_COUNT)
            {
                acc->batch.acc[acc->accCount][0] = sample.vector.x;
                acc->batch.acc[acc->accCount][1] = sample.vector.y;
                acc->batch.acc[acc->accCount][2] = sample.vector.z;
                acc->accCount++;
            }
            break;
        case IMU_SAMPLE_GYR:
            if (acc->gyrCount < IMU_SAMPLE_COUNT)
            {
                acc->batch.gyr[acc->gyrCount][0] = sample.vector.x;
                acc->batch.gyr[acc->gyrCount][1] = sample.vector.y;
                acc->batch.gyr[acc->gyrCount][2] = sample.vector.z;
                acc->gyrCount++;
            }
            break;
        case IMU_SAMPLE_MAG:
            acc->batch.mag[0] = sample.vector.x;
            acc->batch.mag[1] = sample.vector.y;
            acc->batch.mag[2] = sample.vector.z;
            acc->magReady = true;
            break;
        case IMU_SAMPLE_QUAT:
            if (acc->quatCount < IMU_SAMPLE_COUNT)
            {
                acc->batch.quat[acc->quatCount] = sample.quaternion.orientation;
                acc->batch.accuracy = sample.quaternion.accuracy;
                acc->quatCount++;
            }
            break;
        case IMU_SAMPLE_OVERFLOW:
            memset(acc, 0, sizeof(*acc));
            break;
        default:
            break;
    }

    return acc->accCount >= IMU_SAMPLE_COUNT && acc->gyrCount >= IMU_SAMPLE_COUNT &&
           acc->quatCount >= IMU_SAMPLE_COUNT && acc->magReady;
}

static void sendAndReset(ImuBatchAccumulator_t *acc)
{
    if (ComItf_sendImuData(&acc->batch) != E_SUCCESS)
    {
        NAQILOG_WARN("IMU packet transmission failed\r");
    }

    memset(acc, 0, sizeof(ImuBatchAccumulator_t));
}

int8_t Imu_initialization(mxc_i2c_regs_t *i2c_inst)
{
    return ImuItf_deviceInit(i2c_inst);
}

/**
 * @brief   Callback registered with ImuItf_enableDataReadyInterrupt(), invoked
 *          from ISR context on each BHI160 data-ready interrupt. Owns all
 *          FreeRTOS-specific notification logic — the interface layer below
 *          has none.
 * @return  None.
 */
static void imu_notifyDataReady(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (IMUAcquisitionHandle)
    {
        xTaskNotifyFromISR(IMUAcquisitionHandle, IMU_DATA_READY_BIT, eSetBits, &xHigherPriorityTaskWoken);
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/**
 * @brief Function implementing the imu acquisition thread.
 *
 * @param pvParameters Not used
 * @retval None
 */
void Imu_acquireDataTask(void *pvParameters)
{
    ImuBatchAccumulator_t accumulator;
    ImuSampleEvent_t      sample;

    (void)pvParameters;

    NAQILOG_INFO("IMU task launched");

    IMUAcquisitionHandle = xTaskGetCurrentTaskHandle();

    ImuItf_enableDataReadyInterrupt(imu_notifyDataReady);

    memset(&accumulator, 0, sizeof(ImuBatchAccumulator_t));

    for (;;)
    {
        xTaskNotifyWait(pdFALSE, ULONG_MAX, NULL, portMAX_DELAY);

        while (ImuItf_readNextSample(&sample))
        {
            if (accumulateSample(&accumulator, sample))
            {
                sendAndReset(&accumulator);
            }
        }
    }
}
