/**
 * @file main.c
 * @author pierre@wisear.io
 * @brief
 * @version 0.1
 * @date 2026-01-15
 *
 * @copyright Copyright (c) 2026
 *
 */

#include "logger.h"
#include "mxc_device.h"
#include "SystemServices.h"
#include "SPI_Driver.h"
#include "Communication.h"
#include "i2c_driver.h"
#include "board_init.h"
#include "board.h"

/* Stringification macros */
#define STRING(x) STRING_(x)
#define STRING_(x) #x


/*
 * Function Prototype declarations
 *
 */

/**
 * @brief Program entry point
 *
 * @return int
 */
int main(void)
{
    /* Delay to prevent bricks */
    for (volatile int i = 0; i < 0xFFFFFF; i++)
        ;

    Gpio_initialization();

	/* Initialize the UART0 to communicate with QCC5141 */
	UART_Initialization(UART0);

	/* Initialize the SPI0 to acquire data from ExG */
    SPI_Initialization(MXC_SPI0);

    /* Initialize I2C peripheral */
    I2C_Initialization(MXC_I2C1);

    Sys_start();

    /* This code is only reached if the scheduler failed to start */
    NAQILOG_ERROR("ERROR: FreeRTOS did not start due to above error!");
    while (1)
    {
        __NOP();
    }

    /* Quiet GCC warnings */
    return -1;
}
