#include "processing_jaw_clenching.h"
#include "WindowClassifier.h"
#include "models/jc_model.h"
#include "processing_utils.h"

#define TF_ARENA_SIZE   (5 * 1024)

#define SIZE_WINDOW_MVA 31

static WindowClassifier        *pJC_WindowClassifier = NULL;
static tflite::MicroOpResolver *micro_op_resolver;

static void add_ops_wrapper()
{
    jc_model_add_ops(*(tflite::MicroMutableOpResolver<JC_MODEL_NUMBER_OF_OPERATIONS> *)micro_op_resolver);
}

int8_t jc_processing_initialize()
{
    // Resolver
    micro_op_resolver = new tflite::MicroMutableOpResolver<JC_MODEL_NUMBER_OF_OPERATIONS>;

    pJC_WindowClassifier = new WindowClassifier(TF_ARENA_SIZE, JC_MODEL_NB_CLASSES, JC_MODEL_QUANTIZE_INPUT, jc_model,
                                                micro_op_resolver, add_ops_wrapper);

    return pJC_WindowClassifier != NULL ? 0 : -1;
}

int8_t jc_processing_runPreprocessing(float *inputData,
                                      size_t sizeInputData,
                                      float *preprocessedWindow,
                                      size_t sizePreprocessedWindow)
{
    int8_t iRetCode = -1;

    if (sizePreprocessedWindow + JC_PROCESSING_SIZE_EDGE_CROPPING_LEFT < sizeInputData)
    {
        /* Crop for side effects */
        memcpy(preprocessedWindow, &inputData[JC_PROCESSING_SIZE_EDGE_CROPPING_LEFT],
               sizePreprocessedWindow * sizeof(float));

        iRetCode = 0;
    }

    return iRetCode;
}

uint8_t jc_processing_runPrediction(float *preprocessedWindow, size_t sizePreprocessedWindow)
{
    // Scaling
    processing_utils_scale_window(preprocessedWindow, sizePreprocessedWindow);
    // Moving average
    processing_utils_compute_moving_average(preprocessedWindow, sizePreprocessedWindow, SIZE_WINDOW_MVA);
    // Running prediction
    return pJC_WindowClassifier->run_inference(preprocessedWindow, sizePreprocessedWindow);
}
