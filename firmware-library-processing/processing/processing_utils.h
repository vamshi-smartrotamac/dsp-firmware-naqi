/*
 * processing_utils.h
 *
 *  Created on: Feb 14, 2023
 *      Author: pierred
 */

#ifndef PROCESSING_UTILS_H
#define PROCESSING_UTILS_H

#define PROCESSING_UTILS_CEILING(x, y) (((x) + (y)-1) / (y))  // This ceiling function only works for positive values
#define PROCESSING_UTILS_MAX(X, Y)     (((X) > (Y)) ? (X) : (Y))

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#include <stdint.h>
#include <stdlib.h>

    void processing_utils_compute_moving_average(float *inputData, size_t sizeInputData, size_t sizeMVA);

    void processing_utils_scale_window(float *window, size_t window_size);

    int8_t processing_utils_downsample(float   *data,
                                   uint16_t sizeData,
                                   float   *downsampledWindow,
                                   uint16_t sizeDownsampledWindow,
                                   uint8_t  downsamplingFactor);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  // PROCESSING_UTILS_H
