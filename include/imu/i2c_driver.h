/**************************************************************************
 * Copyright (c) 2025 Naqi Logix Inc. All Rights Reserved.
 *
 * File: i2c_test.h
 *
 * This file is part of the proprietary firmware and is protected by
 * applicable copyright laws and international treaties. Unauthorized use,
 * reproduction, or distribution of this file or its contents is strictly
 * prohibited. For permissions, contact info @ naqilogix.com.
 **************************************************************************/

#ifndef I2C_TEST_H_
#define I2C_TEST_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "board.h"
#include "mxc.h"

#define I2C_FREQ 1000000

int I2C_Initialization(mxc_i2c_regs_t *i2c_inst);

int I2C_write(mxc_i2c_regs_t *i2c_inst, uint16_t dev_addr, uint8_t reg, uint8_t *val, uint8_t size);

int I2C_read(mxc_i2c_regs_t *i2c_inst, uint16_t dev_addr, uint8_t reg, uint8_t *dat, uint32_t size);

#endif /* I2C_TEST_H_ */
