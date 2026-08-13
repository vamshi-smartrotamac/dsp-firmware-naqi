#include "filtering_utils.h"
#include <math.h>
#include <stdio.h>

#define WORK_BUFFER_SIZE   0x700
#define MAX_NUMBER_SECTION 9

#ifndef debug
    #define debug(...) printf
#endif

/* Define a function to reverse an array */
void reverse(float arr[], int sizeBuffer)
{
    int idxInit = 0;
    int idxEnd = sizeBuffer - 1;
    while (idxInit < idxEnd)
    {
        float temp = arr[idxInit];
        arr[idxInit] = arr[idxEnd];
        arr[idxEnd] = temp;
        idxInit++;
        idxEnd--;
    }
}

/* Define a function to handle to logic of the SOS multiplication with the inputs/outputs */
int8_t iirFiltering(float *data, size_t sizeSignal, FilterIIR *filter)
{
    float        inputCurrent, ouputCurrent;
    static float memoryBuffer[MAX_NUMBER_SECTION * FLTR_UTILS_NUMBER_COEFFICIENTS_PER_SECTION];

    if (filter->numberSections > MAX_NUMBER_SECTION)
    {
        debug("Too much filter sections! Increase MAX_NUMBER_SECTION!\n\r");
        return -1;
    }
    else
    {
        // Transform the arrays to 2 dimensional arrays for the syntax buffer[i][j] to be available
        // thus making the code more readable.
        float(*sosSectionsAsMatrix)[FLTR_UTILS_NUMBER_COEFFICIENTS_PER_SECTION] = filter->sosSections;
        float(*initialConditionsAsMatrix)[FLTR_UTILS_NUMBER_COEFFICIENTS_PER_SECTION_INIT_CDT] =
            filter->initialConditions;
        float(*matrixBuffer)[FLTR_UTILS_NUMBER_COEFFICIENTS_PER_SECTION] = memoryBuffer;

        // Initialize memoryBuffer with initial conditions
        for (uint8_t i = 0; i < filter->numberSections; i++)
        {
            for (uint8_t j = 0; j < FLTR_UTILS_NUMBER_COEFFICIENTS_PER_SECTION_INIT_CDT; j++)
            {
                matrixBuffer[i][j] = initialConditionsAsMatrix[i][j] * data[0];
            }
        }

        for (unsigned short i = 0; i < sizeSignal; i++)
        {
            inputCurrent = data[i];

            for (uint8_t s = 0; s < filter->numberSections; s++)
            {
                ouputCurrent = sosSectionsAsMatrix[s][0] * inputCurrent + matrixBuffer[s][0];
                matrixBuffer[s][0] = sosSectionsAsMatrix[s][1] * inputCurrent -
                                     sosSectionsAsMatrix[s][4] * ouputCurrent + matrixBuffer[s][1];
                matrixBuffer[s][1] =
                    sosSectionsAsMatrix[s][2] * inputCurrent - sosSectionsAsMatrix[s][5] * ouputCurrent;
                inputCurrent = ouputCurrent;
            }
            data[i] = inputCurrent;
        }
        return 0;
    }
}

int8_t fltr_utils_iirSosFiltFilt(float *data, size_t sizeData, FilterIIR *filter)
{
    static float filterWorkBuffer[WORK_BUFFER_SIZE];

    /* Apply padding to the signal : this is a traduction of the odd_ext function from scipy, since it is used in
     * MNE for IIR filtering */
    size_t padLen = fmin(filter->padLen, sizeData - 1);
    size_t sizePaddedSignal = 2 * padLen + sizeData;
    if (sizePaddedSignal > WORK_BUFFER_SIZE)
    {
        debug("Signal too large for filter work buffer! Increase WORK_BUFFER_SIZE!\n\r");
        return -1;
    }
    else
    {
        for (unsigned short i = 0; i < sizePaddedSignal; i++)
        {
            if (i < padLen)
            {  // padding on the left
                unsigned short j = padLen - i;
                filterWorkBuffer[i] = 2 * data[0] - data[j];
            }

            if ((i < sizeData + padLen) && (i >= padLen))
            {  // signal in the center
                filterWorkBuffer[i] = data[i - padLen];
            }

            if (i >= sizeData + padLen)
            {  // padding on the right
                unsigned short j = i - (padLen + sizeData);
                filterWorkBuffer[i] = 2 * data[sizeData - 1] - data[sizeData - j - 2];
            }
        }

        /* Forward filtering of the signal */
        if (iirFiltering(filterWorkBuffer, sizePaddedSignal, filter) < 0)
        {
            return -1;
        }

        /* Reverse the signal before second filtering */
        reverse(filterWorkBuffer, sizePaddedSignal);

        /* Reverse filtering */
        if (iirFiltering(filterWorkBuffer, sizePaddedSignal, filter) < 0)
        {
            return -1;
        }

        /* Reverse back the signal */
        reverse(filterWorkBuffer, sizePaddedSignal);

        /* Crop the padding */
        for (unsigned short i = 0; i < sizeData; i++)
        {
            data[i] = filterWorkBuffer[i + padLen];  // for odd padding
        }
        return 0;
    }
}
