/**
 * @file ExGacquisitionInterface.h
 * @author pierre@wisear.io
 * @brief
 * @version 0.1
 * @date 2026-01-19
 *
 * @copyright Copyright (c) 2026
 *
 */

#ifndef EXG_ACQUISITION_INTERFACE_H
#define EXG_ACQUISITION_INTERFACE_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Configure the AFE with the required settings
 *
 * @return Error code
 * @retval '0' - Success
 * @retval '-1' - Error
 */
int8_t AfeItf_configure(void);

/**
 * @brief Trigger AFE continuous data acquisition
 *
 */
void AfeItf_startAcquisition(void);

/**
 * @brief Stop AFE continuous data acquisition
 *
 */
void AfeItf_stopAcquisition(void);

/**
 * @brief Reads AFE sample data from the FIFO and converts it to a signed 32-bit integer.
 *
 * @param sample Pointer where sample is written
 * @return Number of samples read, -1 if failure
 */
int8_t AfeItf_readSample(int32_t *sample);

/**
 * @brief Get the latest AC lead-off reading decoded from the FIFO.
 *
 * @param iValue Pointer where the latest in-phase sample (signed 12-bit) is written
 * @param qValue Pointer where the latest quadrature sample (signed 12-bit) is written
 * @param status Pointer where the AC_LOFF status ('1' lead-off, '0' otherwise) is written
 */
void AfeItf_getLeadOffData(int32_t *iValue, int32_t *qValue, uint8_t *status);

/**
 * @brief This function checks if the difference between the lowest and highest
 * value of the ptrDataBuffer window exceeds the given threshold. threshold is
 * provided in microV and is converted by the interface into ADC count.
 *
 * @param ptrDataBuffer Array of ADC count to be analyzed
 * @param bufferSize Size of the array
 * @param threshold Threshold in microV
 * @return true if peak-to-peak value of the input buffer exceeds the given
 * threshold (in microV), false otherwise
 */
bool AfeItf_isThresholdReached(float *ptrDataBuffer, uint16_t bufferSize, uint32_t threshold);

#endif  // EXG_ACQUISITION_INTERFACE_H
