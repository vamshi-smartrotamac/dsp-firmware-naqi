/*
 * SPI_Test.h
 *
 *  Created on: 27-Jan-2026
 *     Author: Naqi
 */

#ifndef SPI_TEST_H_
#define SPI_TEST_H_

#include <stdbool.h>
#include <stdint.h>
#include "spi.h"

/**
 * @brief Initialize SPI interface with settings
 *
 */
int SPI_Initialization(mxc_spi_regs_t *spi);

/**
 * @brief Reads multiple bytes from FIFO
 *
 */
int SPI_Read_Multibyte(mxc_spi_regs_t *spi, uint8_t reg, uint8_t *val, uint8_t size);

/**
 * @brief Writes multiple bytes from FIFO
 *
 */
int SPI_Write(mxc_spi_regs_t *spi, uint8_t reg, uint8_t val);

#endif /* SPI_TEST_H_ */
