/**
 * @file GestureProcessing.c
 * @author pierre@wisear.io
 * @brief
 * @version 0.1
 * @date 2024-02-08
 *
 * @copyright Copyright (c) 2024
 *
 */

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
#include <string.h>
#include "FreeRTOS.h"
#include "limits.h"
#include "task.h"
#include "timers.h"
#include "queue.h"

#include "ExGacquisition.h"
#include "logger.h"
#include "GestureProcessing.h"
#include "filtering_application.h"
#include "ExGacquisitionInterface.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
#define PEAK_TO_PEAK_THRESHOLD 500 // (uV)
#define SLIDING_WINDOW_STEP (1000 * 25 / 256)    // (ms)
#define FILTERED_BUFFER_SIZE (SLIDING_WINDOW_STEP / SAMPLE_PERIOD)
#define JC_TIMER_EVENT (1 << 0)
#define GESTURE_QUEUE_DEPTH 1

/* Global variables ----------------------------------------------------------*/
static TaskHandle_t processingTaskHandle = NULL;
static TimerHandle_t slidingWindowStepTimerHandle = NULL;
static TimerHandle_t memorySystemTimerHandle = NULL;
static QueueHandle_t xGestureQueue = NULL;
static float arrayPreprocessedData[JC_PROCESSING_SIZE_WINDOW_FINAL];
static bool pausedByMemorySystem = false;

// Number of voters for the defined gestures
static const uint8_t jcConsecutivePredictionTarget[] = {
    0,                                            // Padding value because gestures start at 1
    JC_PROCESSING_CONSECUTIVE_PREDICTIONS_DOUBLE, // JC CLASS DOUBLE (1)
    JC_PROCESSING_CONSECUTIVE_PREDICTIONS_TRIPLE, // JC CLASS TRIPLE (2)
    JC_PROCESSING_CONSECUTIVE_PREDICTIONS_SINGLE  // JC CLASS SINGLE (3)
};

// EXG packet gesture_out mapping
static const uint8_t protocolGestureMap[] = {
    0, // Padding value because gestures start at 1
    2, // JC CLASS DOUBLE (1)
    3, // JC CLASS TRIPLE (2)
    1  // JC CLASS SINGLE (3)
};

static const char* const gestureName[] = {
    "No gesture",
    "DJC",
    "TJC",
    "SJC"
};

/* Private function prototypes -----------------------------------------------*/

/**
 * @brief Routine triggering the periodic execution of prediction algorithms
 *
 * @param xTimer Handle of the timer that triggered the interrupt
 * @retval None
 */
static void slidingWindowStepCallback(TimerHandle_t timerHandle)
{
    if (timerHandle == slidingWindowStepTimerHandle)
    {
        xTaskNotify(processingTaskHandle, JC_TIMER_EVENT, eSetBits);
    }
}

/**
 * @brief Routine triggered to exit memory system delay
 *
 * @param xTimer Handle of the timer that triggered the interrupt
 * @retval None
 */
static void memorySystemCallback(TimerHandle_t timerHandle)
{
    if (timerHandle == memorySystemTimerHandle)
    {
        pausedByMemorySystem = false;
    }
}

/**
 * @brief Run the prediction and the post processing layers (voter system and memory system)
 *
 * @retval None
 */
static void computePrediction(void)
{
    static uint8_t previousPrediction = 0;
    static uint8_t consecutivePredictionsCpt = 0;
    uint8_t currentPrediction = 0;

    // Thresholding
    if (!AfeItf_isThresholdReached(arrayPreprocessedData, JC_PROCESSING_SIZE_WINDOW_FINAL, PEAK_TO_PEAK_THRESHOLD))
    {
        // Prediction
        currentPrediction = jc_processing_runPrediction(arrayPreprocessedData, JC_PROCESSING_SIZE_WINDOW_FINAL);
    }

    if ((currentPrediction != 0) && (currentPrediction == previousPrediction))
    {
        consecutivePredictionsCpt++;

        if (consecutivePredictionsCpt == jcConsecutivePredictionTarget[currentPrediction])
        {
            NAQILOG_DEBUG("%s detected", gestureName[currentPrediction]);

            xQueueOverwrite(xGestureQueue, &protocolGestureMap[currentPrediction]);

            consecutivePredictionsCpt = 1;
            previousPrediction = 0;

            if (xTimerStart(memorySystemTimerHandle, pdMS_TO_TICKS(5)) == pdPASS)
            {
                pausedByMemorySystem = true;
            }
            else
            {
                NAQILOG_ERROR("Failed to start memory system timer\r");
            }
        }
    }
    else
    {
        consecutivePredictionsCpt = 1;
        previousPrediction = currentPrediction;
    }
}

/**
 * @brief Processing task initialization
 *
 * @return int8_t
 */
int8_t Gproc_initProcessing(void)
{
    if (jc_processing_initialize() < 0)
    {
        NAQILOG_ERROR("JC processing initialization failed!\r"); // TODO: Fix condition when JC_initialize return code is consistent
    }

    // Synchronisation resources creation
    slidingWindowStepTimerHandle = xTimerCreate("Sliding window step timer", pdMS_TO_TICKS(SLIDING_WINDOW_STEP), pdTRUE,
                                                NULL, slidingWindowStepCallback);

    if (!slidingWindowStepTimerHandle)
    {
        NAQILOG_ERROR("Failed to create sliding window step timer\r");
        return -1;
    }

    memorySystemTimerHandle = xTimerCreate("Memory system timer", pdMS_TO_TICKS(JC_PROCESSING_MEMORY_SYSTEM_DURATION),
                                           pdFALSE, NULL, memorySystemCallback);

    if (!memorySystemTimerHandle)
    {
        NAQILOG_ERROR("Failed to create memory system timer\r");
        return -1;
    }

    xGestureQueue = xQueueCreate(GESTURE_QUEUE_DEPTH, sizeof(uint8_t));

    if (!xGestureQueue)
    {
        NAQILOG_ERROR("Failed to create gesture queue\r");
        return -1;
    }

    return 0;
}

/**
 * @brief Consume the latest detected gesture, one-shot.
 *
 * @return Gesture value (protocol mapping) to stamp into EXG packet's
 *         gesture_out field, or 0 if none is pending.
 */
uint8_t Gproc_getGesture(void)
{
    uint8_t gesture = 0;

    if (xGestureQueue != NULL)
    {
        xQueueReceive(xGestureQueue, &gesture, 0);
    }

    return gesture;
}

/**
 * @brief Function implementing the Processing thread.
 *
 * @param pvParameters Not used
 * @retval None
 */
void Gproc_runProcessingTask(void *pvParameters)
{
    uint32_t notifiedValue = 0;
    static float arrayInputWindow[JC_PROCESSING_SIZE_WINDOW_INPUT];

    NAQILOG_INFO("Processing task launched\r");

    processingTaskHandle = xTaskGetCurrentTaskHandle();

    if (xTimerStart(slidingWindowStepTimerHandle, pdMS_TO_TICKS(5)) != pdPASS)
    {
        NAQILOG_ERROR("Failed to start sliding window step timer!\r");
    }

    for (;;)
    {
        if (xTaskNotifyWait(pdFALSE,        /* Don't clear bits on entry. */
                            ULONG_MAX,      /* Clear all bits on exit. */
                            &notifiedValue, /* Stores the notified value. */
                            portMAX_DELAY) == pdPASS)
        {
            // If at least one processing feature is enabled
            if (notifiedValue & JC_TIMER_EVENT)
            {
            	Exg_copyFloatAFESamples(arrayInputWindow, JC_PROCESSING_SIZE_WINDOW_INPUT,
                                        JC_PROCESSING_SIZE_WINDOW_INPUT);

                if (fltr_app_runFilteringEMG(arrayInputWindow, JC_PROCESSING_SIZE_WINDOW_INPUT) < 0)
                {
                    NAQILOG_ERROR("Window filtering failed");
                }
                // Preprocessing
                else if (jc_processing_runPreprocessing(arrayInputWindow, JC_PROCESSING_SIZE_WINDOW_INPUT,
                                                   arrayPreprocessedData, JC_PROCESSING_SIZE_WINDOW_FINAL) < 0)
                {
                    NAQILOG_ERROR("JC preprocessing failed!");
                }

                if (!pausedByMemorySystem)
                {
                    // Run JC model
                    computePrediction();
                }
            }
        }
    }
}
