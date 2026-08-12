#ifndef PROCESSING_JAW_CLENCHING_H
#define PROCESSING_JAW_CLENCHING_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stddef.h>
#include <stdint.h>

#define JC_PROCESSING_SIZE_WINDOW_FINAL        513
#define JC_PROCESSING_SIZE_EDGE_CROPPING_LEFT  75
#define JC_PROCESSING_SIZE_EDGE_CROPPING_RIGHT 25
#define JC_PROCESSING_SIZE_WINDOW_INPUT \
    (JC_PROCESSING_SIZE_WINDOW_FINAL + JC_PROCESSING_SIZE_EDGE_CROPPING_LEFT + JC_PROCESSING_SIZE_EDGE_CROPPING_RIGHT)
#define JC_PROCESSING_CONSECUTIVE_PREDICTIONS_SINGLE 8
#define JC_PROCESSING_CONSECUTIVE_PREDICTIONS_DOUBLE 7
#define JC_PROCESSING_CONSECUTIVE_PREDICTIONS_TRIPLE 5
#define JC_PROCESSING_MEMORY_SYSTEM_DURATION         1600
#define JC_PROCESSING_CLASS_DOUBLE                   1
#define JC_PROCESSING_CLASS_TRIPLE                   2
#define JC_PROCESSING_CLASS_SINGLE                   3

    int8_t jc_processing_initialize();

    int8_t jc_processing_runPreprocessing(float *inputData,
                                          size_t sizeInputData,
                                          float *preprocessedWindow,
                                          size_t sizePreprocessedWindow);

    uint8_t jc_processing_runPrediction(float *preprocessedWindow, size_t sizePreprocessedWindow);

#ifdef __cplusplus
}
#endif

#endif  // PROCESSING_JAW_CLENCHING_H
