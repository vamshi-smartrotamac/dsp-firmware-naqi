#ifndef PROCESSING_EYE_MOVEMENT_H
#define PROCESSING_EYE_MOVEMENT_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stddef.h>
#include <stdint.h>
#include <processing_utils.h>

#define EM_PROCESSING_SIZE_WINDOW_POST_CROPPING 331
#define EM_PROCESSING_SIZE_EDGE_CROPPING_LEFT   250
#define EM_PROCESSING_SIZE_EDGE_CROPPING_RIGHT  0
#define EM_PROCESSING_SIZE_WINDOW_INPUT                                                \
    (EM_PROCESSING_SIZE_WINDOW_POST_CROPPING + EM_PROCESSING_SIZE_EDGE_CROPPING_LEFT + \
     EM_PROCESSING_SIZE_EDGE_CROPPING_RIGHT)
#define EM_PROCESSING_DOWNSAMPLING_FACTOR 2
#define EM_PROCESSING_SIZE_WINDOW_FINAL \
    (PROCESSING_UTILS_CEILING(EM_PROCESSING_SIZE_WINDOW_POST_CROPPING, EM_PROCESSING_DOWNSAMPLING_FACTOR))
#define EM_PROCESSING_CONSECUTIVE_PREDICTIONS 2
#define EM_PROCESSING_MEMORY_SYSTEM_DURATION  900
#define EM_PROCESSING_CLASS_SINGLE_LEFT       1
#define EM_PROCESSING_CLASS_SINGLE_RIGHT      2

    int8_t em_processing_initialize();

    int8_t em_processing_runPreprocessing(float *EMGData,
                                          size_t sizeEMGData,
                                          float *EOGData,
                                          size_t sizeEOGData,
                                          float *preprocessedWindow,
                                          size_t sizePreprocessedWindow);

    uint8_t em_processing_runPrediction(float *preprocessedWindow, size_t sizePreprocessedWindow);

#ifdef __cplusplus
}
#endif

#endif  // PROCESSING_EYE_MOVEMENT_H
