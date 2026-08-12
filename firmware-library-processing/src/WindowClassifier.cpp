/*
 * Copyright (c) 2021, CATIE, All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include "WindowClassifier.h"

#include <array>

// Define your trace function here. Ex: #define debug(...) printf(__VA_ARGS__)
#ifndef debug
    #define debug(...) printf
#endif

WindowClassifier::WindowClassifier(const uint16_t           tf_arena_size,
                                   const uint8_t            nb_classes,
                                   const bool               quantize_input,
                                   const unsigned char     *model,
                                   tflite::MicroOpResolver *micro_op_resolver,
                                   void (*add_ops_wrapper)())
    : _nb_classes(nb_classes),
      _quantize_input(quantize_input),
      _tf_arena_size(tf_arena_size),
      _micro_op_resolver(micro_op_resolver),
      _add_ops_wrapper(add_ops_wrapper)
{
    // Allocate Memory
    _tensor_arena = new uint8_t[_tf_arena_size];

    // Load Model
    tfmodel = ::tflite::GetModel(model);

    if (tfmodel->version() != TFLITE_SCHEMA_VERSION)
    {
        debug("Model provided is schema version %ld not equal to supported version %d.\n", tfmodel->version(),
              TFLITE_SCHEMA_VERSION);
    }

    _add_ops_wrapper();

    // Initialize Interpreter
    _interpreter =
        new tflite::MicroInterpreter(tfmodel, *_micro_op_resolver, _tensor_arena, _tf_arena_size, _error_reporter);

    // Allocate Tensors
    if (_interpreter->AllocateTensors() != kTfLiteOk)
    {
        debug("Tensor allocation failed\n");
        exit(-1);
    }

    input = _interpreter->input(0);
    output = _interpreter->output(0);
}

uint8_t WindowClassifier::run_inference(float *preprocessedWindow, size_t sizePreprocessedWindow)
{
    for (uint16_t i = 0; i < sizePreprocessedWindow; i++)
    {
        if (_quantize_input)
        {
            input->data.int8[i] = (int8_t)(preprocessedWindow[i] / input->params.scale + input->params.zero_point);
        }
        else
        {
            input->data.f[i] = preprocessedWindow[i];
        }
    }

    // Run inferencing
    TfLiteStatus invokeStatus = _interpreter->Invoke();

    if (invokeStatus != kTfLiteOk)
    {
        debug("Invoke failed!\n");
        return -1;
    }

    debug("Arena size used: %u bytes \n", _interpreter->arena_used_bytes());

    // Postprocess the prediction from the output
    float inference[_nb_classes];
    for (uint8_t i = 0; i < _nb_classes; i++)
    {
        if (_quantize_input)
        {
            inference[i] = (output->data.int8[i] - output->params.zero_point) * output->params.scale;
        }
        else
        {
            inference[i] = output->data.f[i];
        }
    }

    uint8_t maxIndex = 0;  // Initialize the index of the maximum element as the first element

    for (uint8_t i = 1; i < _nb_classes; i++)
    {
        if (inference[i] >= inference[maxIndex])
        {
            maxIndex = i;  // Update the index of the maximum element
        }
    }

    return maxIndex;  // Return the index of the maximum element
}
