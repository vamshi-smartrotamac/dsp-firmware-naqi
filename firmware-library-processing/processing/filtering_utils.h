/*
 * filtering_utils.h
 *
 *  Created on: Feb 14, 2023
 *      Author: pierred
 */

#ifndef FILTERING_UTILS_H
#define FILTERING_UTILS_H

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#include <stdint.h>
#include <stdlib.h>

#define FLTR_UTILS_NUMBER_COEFFICIENTS_PER_SECTION          6
#define FLTR_UTILS_NUMBER_COEFFICIENTS_PER_SECTION_INIT_CDT 2

    // Filter structure
    typedef struct
    {
        uint8_t numberSections;
        size_t  padLen;
        float  *sosSections;
        float  *initialConditions;
    } FilterIIR;

    int8_t fltr_utils_iirSosFiltFilt(float *data, size_t sizeData, FilterIIR *filter);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  // FILTERING_UTILS_H
