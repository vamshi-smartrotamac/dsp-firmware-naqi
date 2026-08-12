/**
 * @file ExGacquisitionInterface.c
 * @author pierre@wisear.io
 * @brief
 * @version 0.1
 * @date 2026-01-19
 *
 * @copyright Copyright (c) 2026
 *
 */

/* Includes ------------------------------------------------------------------*/
#include "ExGacquisitionInterface.h"
#include <stdint.h>
#include "Max86176.h"
#include "logger.h"

int8_t AfeItf_configure(void)
{
    int8_t retCode = 0;

    if (MAX86176_init(MXC_SPI0) != 0)
    {
        NAQILOG_ERROR("MAX86176 initialization failed!");
        retCode = -1;
    }

    if (MAX86176_configureACLeadoff() != 0)
    {
        NAQILOG_ERROR("AC lead-off configuration failed!");
        retCode = -1;
    }

    if (MAX86176_configureRLD() != 0)
    {
        NAQILOG_ERROR("RLD configuration failed!");
        retCode = -1;
    }

    if (MAX86176_configurePLL() != 0)
    {
        NAQILOG_ERROR("PLL configuration failed!");
        retCode = -1;
    }

    if (MAX86176_configureECGChannel() != 0)
    {
        NAQILOG_ERROR("ECG channel configuration failed!");
        retCode = -1;
    }

    if (MAX86176_configureLeadBias() != 0)
    {
        NAQILOG_ERROR("Lead bias configuration failed!");
        retCode = -1;
    }

    if (MAX86176_flushFifo() != 0)
    {
        NAQILOG_ERROR("FIFO flush failed!");
        retCode = -1;
    }

    return retCode;
}

void AfeItf_startAcquisition(void)
{
    MAX86176_startAcquisition();
}

void AfeItf_stopAcquisition(void)
{
    MAX86176_stopAcquisition();
}

int8_t AfeItf_readSample(int32_t *sample)
{
    return MAX86176_readSample(sample);
}

void AfeItf_getLeadOffData(int32_t *iValue, int32_t *qValue, uint8_t *status)
{
    MAX86176_getLeadOffData(iValue, qValue, status);
}

bool AfeItf_isThresholdReached(float *ptrDataBuffer, uint16_t bufferSize,
                               uint32_t threshold)
{
    float min = ptrDataBuffer[0], max = ptrDataBuffer[0];

    // The threshold is a difference so we need to compute the difference with
    // the ADC count of 0 to get rid of a potential offset.
    threshold = MAX86176_convertMicroVoltToADC(threshold)
            - MAX86176_convertMicroVoltToADC(0);

    for (uint16_t i = 0; i < bufferSize; i++) {
        if (ptrDataBuffer[i] < min) {
            min = ptrDataBuffer[i];
        } else if (ptrDataBuffer[i] > max) {
            max = ptrDataBuffer[i];
        }
    }

    return ((uint32_t) (max - min) > threshold);
}
