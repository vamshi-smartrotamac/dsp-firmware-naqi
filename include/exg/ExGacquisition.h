/**
 * @file ExGacquisition.h
 * @author pierre@wisear.io
 * @brief
 * @version 0.1
 * @date 2023-12-20
 *
 * @copyright Copyright (c) 2024
 *
 */
#ifndef INC_DATA_ACQUISITION_H
#define INC_DATA_ACQUISITION_H

#include <stdint.h>

#define ACQUISITION_TASK_NAME_EXG "ExgAcquisition"

#define SAMPLES_PER_MESSAGE       8

typedef struct
{
    int32_t data[SAMPLES_PER_MESSAGE];
    int32_t leadOffI;
    int32_t leadOffQ;
    uint8_t leadOffStatus;
} ExGSamples_t;

int8_t Exg_initializeAcquisitionAfe(uint16_t acquisitionBufferSizeAFE);

void Exg_acquireDataTask(void *pvParameters);

int8_t Exg_copyFloatAFESamples(float *ptrDataBuffer, uint16_t dataBufferSize, uint16_t sampleCountToCopy);

#endif /* INC_DATA_ACQUISITION_H */
