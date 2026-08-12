#ifndef MAX86176_H
#define MAX86176_H

#include <stdint.h>
#include "spi.h"

/* Exported functions ---------------------------------------------------------*/

/**
 * @brief Initialize the MAX86176 driver: bind the SPI bus and verify the device ID.
 *
 * @param spi SPI bus the device is attached to
 * @return '0' on success, '-1' on failure
 */
int8_t MAX86176_init(mxc_spi_regs_t *spi);

/**
 * @brief Configure the AFE with AC lead off detection settings.
 *
 * @return '0' on success, '-1' on failure
 */
int8_t MAX86176_configureACLeadoff(void);

/**
 * @brief Configure the AFE with RLD settings.
 *
 * @return '0' on success, '-1' on failure
 */
int8_t MAX86176_configureRLD(void);

/**
 * @brief Configure the PLL/clock: internal oscillator, N divider for a 32768Hz timebase.
 *
 * @return '0' on success, '-1' on failure
 */
int8_t MAX86176_configurePLL(void);

/**
 * @brief Configure the ECG analog front end: sample rate, gain, fast recovery.
 *
 * @return '0' on success, '-1' on failure
 */
int8_t MAX86176_configureECGChannel(void);

/**
 * @brief Convert a voltage in microvolts to the corresponding ADC count, using the
 *        gain and full-scale configured by MAX86176_configureECGChannel().
 *
 * @param microVolt Input voltage in microvolts (unsigned magnitude)
 * @return Corresponding ADC count value, saturated to the ADC range
 */
uint32_t MAX86176_convertMicroVoltToADC(uint32_t microVolt);

/**
 * @brief Configure lead bias and ECG input switches.
 *
 * @return '0' on success, '-1' on failure
 */
int8_t MAX86176_configureLeadBias(void);

/**
 * @brief Flush the FIFO so acquisition starts on a clean buffer.
 *
 * @return '0' on success, '-1' on failure
 */
int8_t MAX86176_flushFifo(void);

/**
 * @brief Enable ECG continuous acquisition.
 *
 */
void MAX86176_startAcquisition(void);

/**
 * @brief Disable ECG continuous acquisition.
 *
 */
void MAX86176_stopAcquisition(void);

/**
 * @brief Read and decode the next ECG sample from the FIFO, if any.
 *
 * @param sample Pointer where the decoded sample is written
 * @return '1' if a sample was read, '0' if the FIFO is empty, '-1' on SPI failure
 */
int8_t MAX86176_readSample(int32_t *sample);

/**
 * @brief Get the latest AC lead-off reading decoded from the FIFO.
 *
 * @param iValue Pointer where the latest in-phase sample (signed 12-bit) is written
 * @param qValue Pointer where the latest quadrature sample (signed 12-bit) is written
 * @param status Pointer where the AC_LOFF status ('1' lead-off, '0' otherwise) is written
 */
void MAX86176_getLeadOffData(int32_t *iValue, int32_t *qValue, uint8_t *status);

#endif  // MAX86176_H
