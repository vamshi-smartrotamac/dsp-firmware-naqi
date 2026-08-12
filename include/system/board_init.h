/**
 * @file board_init.h
 * @brief Board-level initialization helpers for MAX78000 FTHR_RevA.
 */

#ifndef BOARD_INIT_H
#define BOARD_INIT_H

/*------------------------------------------------------------------------------------*/
/*GPIO MACROS*/
/*-------------------------------------------------------------------------------------*/
#define MXC_GPIO_PORT0 (MXC_GPIO0)
#define MXC_GPIO_PORT1 (MXC_GPIO1)
#define MXC_GPIO_PORT2 (MXC_GPIO2)
#define MXC_GPIO_PORT3 (MXC_GPIO3)


/* IMU interrupt pin per board. EVALBOARD (defined by project.mk in the debug
 * profile) = eval board; otherwise the P3 production board. */
#if defined(EVALBOARD)
    // Eval board: IMU INT on P0.19, RGB LED on P2.0
    #define IMU_INTERRUPT_PORT  (MXC_GPIO_PORT0)
    #define IMU_INTERRUPT_PIN   (MXC_GPIO_PIN_19)
    #define LED_PORT            (MXC_GPIO_PORT2)
    #define LED_PIN             (MXC_GPIO_PIN_0)
#else
    // P3 production board: IMU INT on P0.3
    #define IMU_INTERRUPT_PORT  (MXC_GPIO_PORT0)
    #define IMU_INTERRUPT_PIN   (MXC_GPIO_PIN_3)
#endif

void Gpio_initialization(void);

#endif /* BOARD_INIT_H */
