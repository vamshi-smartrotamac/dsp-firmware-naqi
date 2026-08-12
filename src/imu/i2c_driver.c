/**************************************************************************
 * Copyright (c) 2025 Naqi Logix Inc. All Rights Reserved.
 *
 * File: i2c.c
 *
 * This file is part of the proprietary firmware and is protected by
 * applicable copyright laws and international treaties. Unauthorized use,
 * reproduction, or distribution of this file or its contents is strictly
 * prohibited. For permissions, contact info @ naqilogix.com.
 **************************************************************************/

#include "i2c_driver.h"
#include <string.h>
#include "FreeRTOS.h"
#include "logger.h"
#include "task.h"

/*
 *---------------------------------------------------------------------------------------------
 * Function:      I2C_Initialization
 * Description:   This function is Initalization of I2C Periperal
 * Parameters:    mxc_i2c_regs_t* i2c_inst                (I2C Instance)
 * return         None
 *  ---------------------------------------------------------------------------------------------
 */

int I2C_Initialization(mxc_i2c_regs_t *i2c_inst)
{
    int ret;

    if (!i2c_inst)
    {
        NAQILOG_ERROR("I2C handle is NULL");
        return E_NULL_PTR;
    }

    ret = MXC_I2C_Init(i2c_inst, 1, 0);
    if (ret != E_NO_ERROR)
    {
        NAQILOG_ERROR("Error initializing I2C");
        return ret;
    }
    MXC_I2C_SetFrequency(i2c_inst, I2C_FREQ);

    NVIC_SetPriority(I2C1_IRQn, 7);

    return ret;
}

/*
 *---------------------------------------------------------------------------------------------
 * Function:      I2C_write_Multibyte
 * Description:   This function is to write Multiple bytes of data to given address with size mentioned
 * Parameters:    [In1] mxc_i2c_regs_t* i2c_inst                (I2C Instance)
 * 				 [In2] uint16_t dev_addr                	   (Device Address)
 * 				 [In3] uint8_t reg                			   (Register Address)
 * 				 [In4] uint8_t *val                			   (Value)
 * 				 [In5] uint8_t size							   (size of the
 *Data) return         Error code
 *  ---------------------------------------------------------------------------------------------
 */
int I2C_write(mxc_i2c_regs_t *i2c_inst, uint16_t dev_addr, uint8_t reg, uint8_t *val, uint8_t size)
{
    uint8_t       buf[size + 1];
    mxc_i2c_req_t i2c_req = {.i2c = i2c_inst, .addr = dev_addr, .rx_len = 0, .callback = NULL, .restart = 0};

    if (!val)
    {
        return E_NULL_PTR;
    }

    buf[0] = reg;
    memcpy(&buf[1], val, size);
    i2c_req.tx_buf = buf;
    i2c_req.tx_len = sizeof(buf);

    portENTER_CRITICAL();
    int ret = MXC_I2C_MasterTransaction(&i2c_req);
    portEXIT_CRITICAL();

    return ret;
}

/*
 *---------------------------------------------------------------------------------------------
 * Function:      I2C_read
 * Description:   This function is to read the data from given address
 * Parameters:    [In1] mxc_i2c_regs_t* i2c_inst                (I2C Instance)
 * 				 [In2] uint16_t dev_addr                	   (Device Address)
 * 				 [In3] uint8_t reg                			   (Register Address)
 * 				 [In4] uint8_t *val                			   (Value)
 * 				 [In5] uint8_t size							   (size of the
 *Data) return         Error code
 *  ---------------------------------------------------------------------------------------------
 */
int I2C_read(mxc_i2c_regs_t *i2c_inst, uint16_t dev_addr, uint8_t reg, uint8_t *dat, uint32_t size)
{
    mxc_i2c_req_t i2c_req = {.i2c = i2c_inst,
                             .addr = dev_addr,
                             .tx_buf = &reg,
                             .tx_len = 1,
                             .rx_buf = dat,
                             .rx_len = size,
                             .callback = NULL,
                             .restart = 0};

    if (!dat)
    {
        return E_NULL_PTR;
    }

    portENTER_CRITICAL();
    int ret = MXC_I2C_MasterTransaction(&i2c_req);
    portEXIT_CRITICAL();

    return ret;
}
