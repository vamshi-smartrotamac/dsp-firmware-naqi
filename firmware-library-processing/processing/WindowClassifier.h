/*
 * Copyright (c) 2021, CATIE, All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef WINDOW_CLASSIFIER_H
#define WINDOW_CLASSIFIER_H

#include "tensorflow-lite-microcontrollers/tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow-lite-microcontrollers/tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow-lite-microcontrollers/tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow-lite-microcontrollers/tensorflow/lite/schema/schema_generated.h"

/*!
 *  \class WindowClassifier
 */
class WindowClassifier
{
  public:
    WindowClassifier(const uint16_t           tf_arena_size,
                     const uint8_t            nb_classes,
                     const bool               quantize_input,
                     const unsigned char     *model,
                     tflite::MicroOpResolver *micro_op_resolver,
                     void (*add_ops_wrapper)());

    uint8_t run_inference(float *preprocessedWindow, size_t sizePreprocessedWindow);

  private:
    // Model constants
    uint8_t  _nb_classes;
    bool     _quantize_input;
    uint16_t _tf_arena_size;

    // Resolver
    tflite::MicroOpResolver *_micro_op_resolver;

    void (*_add_ops_wrapper)();

    // Input and output tensors
    TfLiteTensor *input;
    TfLiteTensor *output;

    // Interpreter
    tflite::MicroInterpreter *_interpreter;

    // Reporter
    tflite::MicroErrorReporter _micro_error_reporter;
    tflite::ErrorReporter     *_error_reporter = &_micro_error_reporter;

    // TF Memory
    uint8_t *_tensor_arena;

    // Model
    const tflite::Model *tfmodel;
};

#endif  // WINDOW_CLASSIFIER_H
