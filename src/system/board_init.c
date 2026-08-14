/**
 * @file board_init.c
 * @author pierre@wisear.io
 * @brief
 * @version 0.1
 * @date 2026-03-31
 *
 * @copyright Copyright (c) 2024
 *
 */

/* Includes ------------------------------------------------------------------*/
#include "board_init.h"
#include "IMUacquisition.h"
#include "board.h"
#include "logger.h"
#include "mxc_errors.h"

/*
 *---------------------------------------------------------------------------------------------
 * Function:      Gpio_initialization
 * Description:   This function is a Initialization function of GPIO Pins
 * Parameters:    None
 * return         None
 * ---------------------------------------------------------------------------------------------
 */
void Gpio_initialization(void)
{
    mxc_gpio_cfg_t imu_interrupt_pin = {
        .port = IMU_INTERRUPT_PORT,
        .mask = IMU_INTERRUPT_PIN,
        .pad = MXC_GPIO_PAD_NONE,
        .func = MXC_GPIO_FUNC_IN,
        .vssel = MXC_GPIO_VSSEL_VDDIOH,
        .drvstr = MXC_GPIO_DRVSTR_0,
    };

    int err = MXC_GPIO_Config(&imu_interrupt_pin);
    if (err != E_NO_ERROR)
    {
        NAQILOG_ERROR("IMU interrupt pin config failed: %d\r", err);
    }
}
