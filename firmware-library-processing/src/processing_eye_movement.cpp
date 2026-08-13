#include "processing_eye_movement.h"
#include "WindowClassifier.h"
#include "models/em_model.h"

#define TF_ARENA_SIZE (5 * 1024)

static WindowClassifier        *pEM_WindowClassifier = NULL;
static tflite::MicroOpResolver *micro_op_resolver;

static void add_ops_wrapper()
{
    em_model_add_ops(*(tflite::MicroMutableOpResolver<EM_MODEL_NUMBER_OF_OPERATIONS> *)micro_op_resolver);
}

int8_t em_processing_initialize()
{
    // Resolver
    micro_op_resolver = new tflite::MicroMutableOpResolver<EM_MODEL_NUMBER_OF_OPERATIONS>;

    pEM_WindowClassifier = new WindowClassifier(TF_ARENA_SIZE, EM_MODEL_NB_CLASSES, EM_MODEL_QUANTIZE_INPUT, em_model,
                                                micro_op_resolver, add_ops_wrapper);

    return pEM_WindowClassifier != NULL ? 0 : -1;
}

int8_t em_processing_runPreprocessing(float *EMGData,
                                      size_t sizeEMGData,
                                      float *EOGData,
                                      size_t sizeEOGData,
                                      float *preprocessedWindow,
                                      size_t sizePreprocessedWindow)
{
    int8_t iRetCode = -1;

    if ((sizeEOGData == sizeEMGData) && (sizePreprocessedWindow + EM_PROCESSING_SIZE_EDGE_CROPPING_LEFT < sizeEOGData))
    {
        /* Adding the EOG and EMG data together to perform parallel filtering. The EOG and EMG buffers were obtained
        through serial filtering, but the final data required by the model needs to contain both frequency bandwidths.*/
        for (unsigned short i = 0; i < sizeEOGData; ++i)
        {
            EOGData[i] += EMGData[i];
        }

        /* Downsample the data according to the chosen downsampling factor for eye movement. The cropping operation
        is also done here as the input chosen for downsampling corresponds to the cropped buffer */
        iRetCode = processing_utils_downsample(&EOGData[EM_PROCESSING_SIZE_EDGE_CROPPING_LEFT],
                                           EM_PROCESSING_SIZE_WINDOW_POST_CROPPING, preprocessedWindow,
                                           sizePreprocessedWindow, EM_PROCESSING_DOWNSAMPLING_FACTOR);
    }

    return iRetCode;
}

uint8_t em_processing_runPrediction(float *preprocessedWindow, size_t sizePreprocessedWindow)
{
    // Scaling
    processing_utils_scale_window(preprocessedWindow, sizePreprocessedWindow);
    // Running prediction
    return pEM_WindowClassifier->run_inference(preprocessedWindow, sizePreprocessedWindow);
}
