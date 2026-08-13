/**
 * @file GestureProcessing.h
 * @author pierre@wisear.io
 * @brief
 * @version 0.1
 * @date 2024-02-08
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef GESTURE_PROCESSING_H
#define GESTURE_PROCESSING_H

#include <stdbool.h>
#include <stdint.h>

#include "processing_jaw_clenching.h"

#define PROCESSING_TASK_NAME "Processing"

int8_t Gproc_initProcessing(void);

void Gproc_runProcessingTask(void *pvParameters);

uint8_t Gproc_getGesture(void);

#endif /* GESTURE_PROCESSING_H */
