#include "processing_utils.h"
#include <math.h>

void processing_utils_scale_window(float *window, size_t window_size)
{
    float sum = 0.0;
    float var = 0.0;

    // Compute the average value of the window
    for (unsigned short i = 0; i < window_size; i++)
    {
        sum += window[i];
    }
    float avg = sum / window_size;

    // Compute the standard deviation value of the window
    for (unsigned short i = 0; i < window_size; i++)
    {
        var += pow(window[i] - avg, 2);
    }
    float sd = sqrt(var / window_size);

    // Scale the window using standard normalization
    for (unsigned short i = 0; i < window_size; i++)
    {
        window[i] = (window[i] - avg) / sd;
    }
}

void processing_utils_compute_moving_average(float *inputData, size_t sizeInputData, size_t sizeMVA)
{
    const size_t padding_length = sizeMVA - 1;
    const size_t padding_right = padding_length / 2;
    const size_t padding_left = padding_length - padding_right;
    float        sum_data = 0;
    float        input_memory_buffer[padding_left + 1];

    // Preparing sum for moving average computation
    for (unsigned short j = 0; j < padding_right; j++)
    {
        sum_data += fabsf(inputData[j]);
    }

    // Computing successive average windows by adding a new member and subtracting the oldest.
    // As the moving average starts at 0, the buffer is only half full at the beginning.
    // So we only remove old values once the index has reached padding_left, and stop adding new
    // values when the index has reached SIZE_WINDOW_FINAL - padding_right.
    for (unsigned short i = 0; i < sizeInputData; i++)
    {
        if (i < padding_left + 1)
        {
            sum_data += fabsf(inputData[i + padding_right]);
        }
        else if (i > padding_left && i < sizeInputData - padding_right)
        {
            sum_data -= fabsf(input_memory_buffer[i % (padding_left + 1)]);
            sum_data += fabsf(inputData[i + padding_right]);
        }
        else
        {
            sum_data -= fabsf(input_memory_buffer[i % (padding_left + 1)]);
        }
        input_memory_buffer[i % (padding_left + 1)] = inputData[i];
        inputData[i] = sum_data / sizeMVA;
    }
}

int8_t processing_utils_downsample(float   *data,
                               uint16_t sizeData,
                               float   *downsampledWindow,
                               uint16_t sizeDownsampledWindow,
                               uint8_t  downsamplingFactor)
{
    int8_t iRetCode = -1;

    if (sizeDownsampledWindow >= ceil((float)sizeData / (float)downsamplingFactor))
    {
        for (uint16_t i = 0, j = 0; i < sizeData; i += downsamplingFactor)
        {
            downsampledWindow[j++] = data[i];
        }
        iRetCode = 0;
    }
    return iRetCode;
}
