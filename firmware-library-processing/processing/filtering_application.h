#ifndef FILTERING_APPLICATION_H
#define FILTERING_APPLICATION_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stddef.h>
#include <stdint.h>

    int8_t fltr_app_runFilteringEMG(float *inputData, size_t sizeInputData);

    int8_t fltr_app_runFilteringEOG(float *inputData, size_t sizeInputData);

#ifdef __cplusplus
}
#endif

#endif  // FILTERING_APPLICATION_H