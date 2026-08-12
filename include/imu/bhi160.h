/**************************************************************************
 * Copyright (c) 2025 Naqi Logix Inc. All Rights Reserved.
 *
 * File: IMU.h
 *
 * This file is part of the proprietary firmware and is protected by
 * applicable copyright laws and international treaties. Unauthorized use,
 * reproduction, or distribution of this file or its contents is strictly
 * prohibited. For permissions, contact info @ naqilogix.com.
 **************************************************************************/

/*
 * BHI160.h
 *
 *  Created on: 21-Nov-2022
 *      Author: NAQI
 */

#ifndef IMU_H_
#define IMU_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "gpio.h"
#include "i2c.h"

#define STANDARD                                               1
#define ALTERNATIVE                                            0
#define I2C_EEPROM_ADDRESS                                     0x50

#define BHY_INIT_VALUE                                         (0)
#define BHY_GEN_READ_WRITE_LENGTH                              (1)
#define BHY_BYTES_REMAINING_LENGTH                             (2)
#define BHY_CRC_HOST_LENGTH                                    (4)
#define BHY_PARAMETER_ACK_LENGTH                               (250)
#define BHY_READ_BUFFER_LENGTH                                 (16)
#define BHY_FIRMWARE_BOOT_DELAY_MS                             (50)
#define BHY_I2C_XFER_DELAY_US                                  (100)
#define BHY_SIGNATURE_MEM_LEN                                  (17)

#define BHY_ROM_VERSION_DI01                                   (uint16_t)(0x2112)
#define BHY_ROM_VERSION_DI03                                   (uint16_t)(0x2DAD)

/***************************************************************/
/**\name    BIT SHIFTING DEFINITION      */
/***************************************************************/
#define BHY_SHIFT_BIT_POSITION_BY_01_BIT                       (1)
#define BHY_SHIFT_BIT_POSITION_BY_02_BITS                      (2)
#define BHY_SHIFT_BIT_POSITION_BY_03_BITS                      (3)
#define BHY_SHIFT_BIT_POSITION_BY_04_BITS                      (4)
#define BHY_SHIFT_BIT_POSITION_BY_05_BITS                      (5)
#define BHY_SHIFT_BIT_POSITION_BY_06_BITS                      (6)
#define BHY_SHIFT_BIT_POSITION_BY_07_BITS                      (7)
#define BHY_SHIFT_BIT_POSITION_BY_08_BITS                      (8)
#define BHY_SHIFT_BIT_POSITION_BY_16_BITS                      (16)
#define BHY_SHIFT_BIT_POSITION_BY_24_BITS                      (24)
/****************************************************/
/**\name    ARRAY SIZE DEFINITIONS      */
/***************************************************/
#define BHY_BYTES_REMAINING_SIZE                               (2)
#define BHY_BYTES_REMAINING_LSB                                (0)
#define BHY_BYTES_REMAINING_MSB                                (1)

#define BHY_CRC_HOST_SIZE                                      (4)
#define BHY_CRC_HOST_LSB                                       (0)
#define BHY_CRC_HOST_XLSB                                      (1)
#define BHY_CRC_HOST_XXLSB                                     (2)
#define BHY_CRC_HOST_MSB                                       (3)
#define BHY_CRC_HOST_FILE_LSB                                  (4)
#define BHY_CRC_HOST_FILE_XLSB                                 (5)
#define BHY_CRC_HOST_FILE_XXLSB                                (6)
#define BHY_CRC_HOST_FILE_MSB                                  (7)

#define BHY_INDEX_LEN                                          (19)

#define BHY_READ_BUFFER_SIZE                                   (16)
#define BHY_READ_BUFFER_1_REG                                  (0)
#define BHY_READ_BUFFER_2_REG                                  (1)
#define BHY_READ_BUFFER_3_REG                                  (2)
#define BHY_READ_BUFFER_4_REG                                  (3)
#define BHY_READ_BUFFER_5_REG                                  (4)
#define BHY_READ_BUFFER_6_REG                                  (5)
#define BHY_READ_BUFFER_7_REG                                  (6)
#define BHY_READ_BUFFER_8_REG                                  (7)
#define BHY_READ_BUFFER_9_REG                                  (8)
#define BHY_READ_BUFFER_10_REG                                 (9)
#define BHY_READ_BUFFER_11_REG                                 (10)
#define BHY_READ_BUFFER_12_REG                                 (11)
#define BHY_READ_BUFFER_13_REG                                 (12)
#define BHY_READ_BUFFER_14_REG                                 (13)
#define BHY_READ_BUFFER_15_REG                                 (14)
#define BHY_READ_BUFFER_16_REG                                 (15)

#define BHY_I2C_REG_BUFFER_ZERO_ADDR                           (0x00)
#define BHY_I2C_REG_BUFFER_END_ADDR                            (0x31)
#define BHY_I2C_REG_BUFFER_LENGTH                              ((BHY_I2C_REG_BUFFER_END_ADDR) - (BHY_I2C_REG_BUFFER_ZERO_ADDR) + 1)
/* fifo flush, chip control and status registers*/
#define BHY_I2C_REG_FIFO_FLUSH_ADDR                            (0x32)
#define BHY_I2C_REG_CHIP_CONTROL_ADDR                          (0x34)
#define BHY_I2C_REG_HOST_STATUS_ADDR                           (0x35)
#define BHY_I2C_REG_INT_STATUS_ADDR                            (0x36)
#define BHY_I2C_REG_CHIP_STATUS_ADDR                           (0x37)
/* bytes remaining register*/
#define BHY_I2C_REG_BYTES_REMAINING_LSB_ADDR                   (0x38)
#define BHY_I2C_REG_BYTES_REMAINING_MSB_ADDR                   (0x39)
#define BHY_I2C_REG_PARAMETER_ACKNOWLEDGE_ADDR                 (0x3A)
/* saved parameter */
#define BHY_I2C_REG_PARAMETER_READ_BUFFER_ZERO                 (0x3B)

#define BHY_I2C_REG_PARAMETER_PAGE_SELECT_ADDR                 (0x54)
/**< parameter page selection address*/
#define BHY_I2C_REG_HOST_INTERFACE_CONTROL_ADDR                (0x55)
/**< host interface control address*/
/*!
 * @brief parameter write buffer*/
#define BHY_I2C_REG_PARAMETER_WRITE_BUFFER_ZERO                (0x5C)
#define BHY_I2C_REG_PARAMETER_REQUEST_ADDR                     (0x64)
/**< used to select the respective parameter number*/

#define BHY_BHY_HOST_IRQ_TIMESTAMP_ADDR                        (0x6C)
/**< host IRQ time stamp address*/
#define BHY_ROM_VERSION_ADDR                                   (0x70)
/**< ROM version address*/
#define BHY_RAM_VERSION_ADDR                                   (0x72)
/**< RAM version address*/
#define BHY_I2C_REG_PRODUCT_ID_ADDR                            (0x90)
/**< product id address*/
#define BHY_I2C_REG_REVISION_ID_ADDR                           (0x91)
/**< revision id address*/
#define BHY_I2C_REG_UPLOAD_0_ADDR                              (0x94)
#define BHY_I2C_REG_UPLOAD_1_ADDR                              (0x95)
/**< upload address for RAM patch*/
#define BHY_I2C_REG_UPLOAD_DATA_ADDR                           (0x96)
/**< upload data address for RAM patch*/
#define BHY_I2C_REG_CRC_HOST_ADDR                              (0x97)
/**< CRC register*/
#define BHY_I2C_REG_RESET_REQUEST_ADDR                         (0x9B)

#define BHY_SENSOR_BANK_STATUS_DATA_AVAILABLE                  (0x01)
#define BHY_SENSOR_BANK_STATUS_I2C_NACK                        (0x02)
#define BHY_SENSOR_BANK_STATUS_DEVICE_ID_ERROR                 (0x04)
#define BHY_SENSOR_BANK_STATUS_TRANSIENT_ERROR                 (0x08)
#define BHY_SENSOR_BANK_STATUS_DATA_LOST                       (0x10)
#define BHY_SENSOR_BANK_STATUS_POWER_MODE                      (0xE0)
#define BHY_POWER_MODE_SENSOR_NOT_PRESENT                      (0x00)
#define BHY_POWER_MODE_POWER_DOWN                              (0x01)
#define BHY_POWER_MODE_POWER_SUSPEND                           (0x02)
#define BHY_POWER_MODE_POWER_SELFTEST                          (0x03)
#define BHY_POWER_MODE_POWER_INTR_MOTION                       (0x04)
#define BHY_POWER_MODE_POWER_ONE_SHOT                          (0x05)
#define BHY_POWER_MODE_POWER_LOW_POWER                         (0x06)
#define BHY_POWER_MODE_POWER_ACTIVE                            (0x07)
#define BHY_SIGNATURE_1                                        (0)
#define BHY_SIGNATURE_2                                        (1)
#define BHY_SIG_FLAG_1_POS                                     (2)
#define BHY_SIG_FLAG_2_POS                                     (3)
#define BHY_SIGNATURE_LENGTH                                   (16)
#define BHY_RAM_WRITE_LENGTH                                   (4)
#define BHY_RAM_WRITE_LENGTH_API                               (32)
#define BHY_CHIP_CTRL_ENABLE_1                                 (0x02)
#define BHY_CHIP_CTRL_ENABLE_2                                 (0x01)
#define BHY_UPLOAD_DATA                                        (0x00)
#define BHY_RESET_ENABLE                                       (0x01)
#define BHY_INIT_READ_BYTES                                    (19)
#define BHY_INIT_BYTE_MINUS_ONE                                (1)
#define BHY_CHECK_BYTE                                         (3)
#define BHY_IMAGE_SIGNATURE1                                   (0x2A)
#define BHY_IMAGE_SIGNATURE2                                   (0x65)
#define BHY_GET_ROMVEREXP(bhy_sig_flag)                        ((bhy_sig_flag >> 11) & 0x03)
#define BHY_ROM_VER_ANY                                        (0x00)
#define BHY_ROM_VER_DI01                                       (0x01)
#define BHY_ROM_VER_DI02                                       (0x02)
#define BHY_ROM_VER_DI03                                       (0x03)

#define BHY_HOST_IRQ_TIMESTAMP_SIZE                            (4)
#define BHY_HOST_IRQ_TIMESTAMP_LSB_DATA                        (0)
#define BHY_HOST_IRQ_TIMESTAMP_XLSB_DATA                       (1)
#define BHY_HOST_IRQ_TIMESTAMP_XXLSB_DATA                      (2)
#define BHY_HOST_IRQ_TIMESTAMP_MSB_DATA                        (3)
/****************************************************/
/**\name ARRAY DEFINITIONS FOR ROM VERSION*/
/**************************************************************/
#define BHY_ROM_VERSION_SIZE                                   (2)
#define BHY_ROM_VERSION_LSB_DATA                               (0)
#define BHY_ROM_VERSION_MSB_DATA                               (1)

#define BHY_RAM_VERSION_SIZE                                   (2)
#define BHY_RAM_VERSION_LSB_DATA                               (0)
#define BHY_RAM_VERSION_MSB_DATA                               (1)
#define BHY_FIFO_DATA_BUFFER                                   (200)

#define BHY_I2C_REG_FIFO_FLUSH__POS                            (0)
#define BHY_I2C_REG_FIFO_FLUSH__MSK                            (0xFF)
#define BHY_I2C_REG_FIFO_FLUSH__LEN                            (8)
#define BHY_I2C_REG_FIFO_FLUSH__REG                            (BHY_I2C_REG_FIFO_FLUSH_ADDR)

/* Abort Transfer: discards the FIFO backlog already announced to the host
 * and resets Bytes Remaining to 0 */
#define BHY_I2C_REG_HOST_INTERFACE_CONTROL_ABORT_TRANSFER__POS (1)
#define BHY_I2C_REG_HOST_INTERFACE_CONTROL_ABORT_TRANSFER__MSK (0x02)
#define BHY_I2C_REG_HOST_INTERFACE_CONTROL_ABORT_TRANSFER__LEN (1)
#define BHY_I2C_REG_HOST_INTERFACE_CONTROL_ABORT_TRANSFER__REG (BHY_I2C_REG_HOST_INTERFACE_CONTROL_ADDR)

#define BHY_I2C_REG_PARAMETER_PAGE_SELECT__POS                 (0)
#define BHY_I2C_REG_PARAMETER_PAGE_SELECT__MSK                 (0xFF)
#define BHY_I2C_REG_PARAMETER_PAGE_SELECT__LEN                 (8)
#define BHY_I2C_REG_PARAMETER_PAGE_SELECT__REG                 (BHY_I2C_REG_PARAMETER_PAGE_SELECT_ADDR)

#define BHY_I2C_REG_PARAMETER_PAGE_SELECT_PARAMETER_PAGE__POS  (0)
#define BHY_I2C_REG_PARAMETER_PAGE_SELECT_PARAMETER_PAGE__MSK  (0x0F)
#define BHY_I2C_REG_PARAMETER_PAGE_SELECT_PARAMETER_PAGE__LEN  (4)
#define BHY_I2C_REG_PARAMETER_PAGE_SELECT_PARAMETER_PAGE__REG  (BHY_I2C_REG_PARAMETER_PAGE_SELECT_ADDR)

#define BHY_I2C_REG_PARAMETER_PAGE_SELECT_PARAMETER_SIZE__POS  (4)
#define BHY_I2C_REG_PARAMETER_PAGE_SELECT_PARAMETER_SIZE__MSK  (0xF0)
#define BHY_I2C_REG_PARAMETER_PAGE_SELECT_PARAMETER_SIZE__LEN  (4)
#define BHY_I2C_REG_PARAMETER_PAGE_SELECT_PARAMETER_SIZE__REG  (BHY_I2C_REG_PARAMETER_PAGE_SELECT_ADDR)

#define BHY_I2C_REG_LOAD_PARAMETER_REQUEST__POS                (0)
#define BHY_I2C_REG_LOAD_PARAMETER_REQUEST__MSK                (0xFF)
#define BHY_I2C_REG_LOAD_PARAMETER_REQUEST__LEN                (8)
#define BHY_I2C_REG_LOAD_PARAMETER_REQUEST__REG                (BHY_I2C_REG_PARAMETER_REQUEST_ADDR)

#define BHY_I2C_REG_RESET_REQUEST__POS                         (0)
#define BHY_I2C_REG_RESET_REQUEST__MSK                         (0xFF)
#define BHY_I2C_REG_RESET_REQUEST__LEN                         (8)
#define BHY_I2C_REG_RESET_REQUEST__REG                         (BHY_I2C_REG_RESET_REQUEST_ADDR)

#define BHY_PAGE_1                                             (0x01)
/**< page 1 system page*/
#define BHY_PAGE_2                                             (0x02)
/**< page 2 system page*/
#define BHY_PAGE_3                                             (0x03)
/**< page 3 sensor page*/
#define BHY_PAGE_15                                            (0x0F)
/**< page 15 sensor page*/

typedef signed char      s8;  /**< used for signed 8bit */
typedef signed short int s16; /**< used for signed 16bit */
typedef signed int       s32; /**< used for signed 32bit */

/*unsigned integer types*/
typedef unsigned char      u8;  /**< used for unsigned 8bit */
typedef unsigned short int u16; /**< used for unsigned 16bit */
typedef unsigned int       u32; /**< used for unsigned 32bit */

#define BHY_RETURN_FUNCTION_TYPE (int8_t)
#define BHY_NULL_PTR             ((void *)0)

#define BHY_WR_FUNC_PTR          s8 (*bus_write)(mxc_i2c_regs_t *, uint8_t, uint8_t, uint8_t *, uint16_t)
#define BHY_RD_FUNC_PTR          s8 (*bus_read)(mxc_i2c_regs_t *, uint8_t, uint8_t, uint8_t *, uint16_t)
#define BHY_BRD_FUNC_PTR         s8 (*burst_read)(mxc_i2c_regs_t *, uint8_t, uint8_t, uint8_t *, uint32_t)

#define BHY_BUS_READ_FUNC(inst, device_addr, reg_addr, reg_data, r_len) \
    bus_read(inst, device_addr, reg_addr, reg_data, r_len)

#define BHY_BURST_READ_FUNC(inst, device_addr, register_addr, register_data, rd_len) \
    burst_read(inst, device_addr, register_addr, register_data, rd_len)

#define BHY_BUS_WRITE_FUNC(inst, device_addr, reg_addr, reg_data, wr_len) \
    bus_write(inst, device_addr, reg_addr, reg_data, wr_len)

#define VS_ID_PADDING                               0
#define VS_ID_ACCELEROMETER                         1
#define VS_ID_MAGNETOMETER                          2
#define VS_ID_ORIENTATION                           3
#define VS_ID_GYROSCOPE                             4
#define VS_ID_LIGHT                                 5
#define VS_ID_BAROMETER                             6
#define VS_ID_TEMPERATURE                           7
#define VS_ID_PROXIMITY                             8
#define VS_ID_GRAVITY                               9
#define VS_ID_LINEAR_ACCELERATION                   10
#define VS_ID_ROTATION_VECTOR                       11
#define VS_ID_HUMIDITY                              12
#define VS_ID_AMBIENT_TEMPERATURE                   13
#define VS_ID_UNCALIBRATED_MAGNETOMETER             14
#define VS_ID_GAME_ROTATION_VECTOR                  15
#define VS_ID_UNCALIBRATED_GYROSCOPE                16
#define VS_ID_SIGNIFICANT_MOTION                    17
#define VS_ID_STEP_DETECTOR                         18
#define VS_ID_STEP_COUNTER                          19
#define VS_ID_GEOMAGNETIC_ROTATION_VECTOR           20
#define VS_ID_HEART_RATE                            21
#define VS_ID_TILT_DETECTOR                         22
#define VS_ID_WAKE_GESTURE                          23
#define VS_ID_GLANCE_GESTURE                        24
#define VS_ID_PICKUP_GESTURE                        25
#define VS_ID_CUS1                                  26
#define VS_ID_CUS2                                  27
#define VS_ID_CUS3                                  28
#define VS_ID_CUS4                                  29
#define VS_ID_CUS5                                  30
#define VS_ID_ACTIVITY                              31

#define BHY_MDELAY_DATA_TYPE                        uint32_t

#define VS_ID_ACCELEROMETER_WAKEUP                  (VS_ID_ACCELEROMETER + 32)
#define VS_ID_MAGNETOMETER_WAKEUP                   (VS_ID_MAGNETOMETER + 32)
#define VS_ID_ORIENTATION_WAKEUP                    (VS_ID_ORIENTATION + 32)
#define VS_ID_GYROSCOPE_WAKEUP                      (VS_ID_GYROSCOPE + 32)
#define VS_ID_LIGHT_WAKEUP                          (VS_ID_LIGHT + 32)
#define VS_ID_BAROMETER_WAKEUP                      (VS_ID_BAROMETER + 32)
#define VS_ID_TEMPERATURE_WAKEUP                    (VS_ID_TEMPERATURE + 32)
#define VS_ID_PROXIMITY_WAKEUP                      (VS_ID_PROXIMITY + 32)
#define VS_ID_GRAVITY_WAKEUP                        (VS_ID_GRAVITY + 32)
#define VS_ID_LINEAR_ACCELERATION_WAKEUP            (VS_ID_LINEAR_ACCELERATION + 32)
#define VS_ID_ROTATION_VECTOR_WAKEUP                (VS_ID_ROTATION_VECTOR + 32)
#define VS_ID_HUMIDITY_WAKEUP                       (VS_ID_HUMIDITY + 32)
#define VS_ID_AMBIENT_TEMPERATURE_WAKEUP            (VS_ID_AMBIENT_TEMPERATURE + 32)
#define VS_ID_UNCALIBRATED_MAGNETOMETER_WAKEUP      (VS_ID_UNCALIBRATED_MAGNETOMETER + 32)
#define VS_ID_GAME_ROTATION_VECTOR_WAKEUP           (VS_ID_GAME_ROTATION_VECTOR + 32)
#define VS_ID_UNCALIBRATED_GYROSCOPE_WAKEUP         (VS_ID_UNCALIBRATED_GYROSCOPE + 32)
#define VS_ID_SIGNIFICANT_MOTION_WAKEUP             (VS_ID_SIGNIFICANT_MOTION + 32)
#define VS_ID_STEP_DETECTOR_WAKEUP                  (VS_ID_STEP_DETECTOR + 32)
#define VS_ID_STEP_COUNTER_WAKEUP                   (VS_ID_STEP_COUNTER + 32)
#define VS_ID_GEOMAGNETIC_ROTATION_VECTOR_WAKEUP    (VS_ID_GEOMAGNETIC_ROTATION_VECTOR + 32)
#define VS_ID_HEART_RATE_WAKEUP                     (VS_ID_HEART_RATE + 32)
#define VS_ID_TILT_DETECTOR_WAKEUP                  (VS_ID_TILT_DETECTOR + 32)
#define VS_ID_WAKE_GESTURE_WAKEUP                   (VS_ID_WAKE_GESTURE + 32)
#define VS_ID_GLANCE_GESTURE_WAKEUP                 (VS_ID_GLANCE_GESTURE + 32)
#define VS_ID_PICKUP_GESTURE_WAKEUP                 (VS_ID_PICKUP_GESTURE + 32)
#define VS_ID_CUS1_WAKEUP                           (VS_ID_CUS1 + 32)
#define VS_ID_CUS2_WAKEUP                           (VS_ID_CUS2 + 32)
#define VS_ID_CUS3_WAKEUP                           (VS_ID_CUS3 + 32)
#define VS_ID_CUS4_WAKEUP                           (VS_ID_CUS4 + 32)
#define VS_ID_CUS5_WAKEUP                           (VS_ID_CUS5 + 32)
#define VS_ID_ACTIVITY_WAKEUP                       (VS_ID_ACTIVITY + 32)

#define VS_ID_DEBUG                                 245
#define VS_ID_TIMESTAMP_LSW_WAKEUP                  246
#define VS_ID_TIMESTAMP_MSW_WAKEUP                  247
#define VS_ID_META_EVENT_WAKEUP                     248
#define VS_ID_BSX_C                                 249
#define VS_ID_BSX_B                                 250
#define VS_ID_BSX_A                                 251
#define VS_ID_TIMESTAMP_LSW                         252
#define VS_ID_TIMESTAMP_MSW                         253
#define VS_ID_META_EVENT                            254

#define BHY_SUCCESS                                 ((uint8_t)0)
#define BHY_NULL                                    ((uint8_t)0)
#define BHY_COMM_RES                                ((int8_t)-1)
#define BHY_OUT_OF_RANGE                            ((int8_t)-2)
#define BHY_ERROR                                   ((int8_t)-3)
#define BHY_RAMPATCH_NOT_MATCH                      ((int8_t)-4)
#define BHY_RAMPATCH_NOT_SUPPORT                    ((int8_t)-5)
#define BHY_CRC_ERROR                               ((int8_t)-6)
#define BHY_PRODUCT_ID_ERROR                        ((int8_t)-7)
#define BHY_DATA_LOST                               ((int8_t)-8)

#define RETRY_NUM                                   (3)
#define PRODUCT_ID_7183                             (0x83)

#define MAX_PAGE_NUM                                15
#define MAX_SENSOR_ID                               0x20
#define MAX_SENSOR_ID_NONWAKEUP                     0x3F
#define MAX_WRITE_BYTES                             8
#define SENSOR_CALLBACK_LIST_NUM                    64
#define TIMESTAMP_CALLBACK_LIST_NUM                 2
#define METAEVENT_CALLBACK_LIST_NUM                 32
#define SENSOR_PARAMETER_WRITE                      0xC0
#define MAX_METAEVENT_ID                            17

#define VS_NON_WAKEUP                               0
#define VS_WAKEUP                                   32
#define VS_FLUSH_NONE                               0x00
#define VS_FLUSH_ALL                                0xFF
#define VS_FLUSH_SINGLE                             0x01

#define BHY_PARAMETER_ACK_CHECK                     (0x80)
#define BHY_MASK_LSB_DATA                           (0x00FF)
#define BHY_MASK_MSB_DATA                           (0xFF00)
#define BHY_SIC_MASK_MSB_DATA                       (0x000000FF)
#define BHY_SIC_MASK_LSB_DATA                       (0x0000FF00)
#define BHY_SIC_MASK_LSB1_DATA                      (0x00FF0000)
#define BHY_SIC_MASK_LSB2_DATA                      (0xFF000000)
#define BHY_MASK_META_EVENT                         (0xFF)

#define BHY_WRITE_BUFFER_SIZE                       (8)
#define BHY_WRITE_BUFFER_1_REG                      (0)
#define BHY_WRITE_BUFFER_2_REG                      (1)
#define BHY_WRITE_BUFFER_3_REG                      (2)
#define BHY_WRITE_BUFFER_4_REG                      (3)
#define BHY_WRITE_BUFFER_5_REG                      (4)
#define BHY_WRITE_BUFFER_6_REG                      (5)
#define BHY_WRITE_BUFFER_7_REG                      (6)
#define BHY_WRITE_BUFFER_8_REG                      (7)

#define BHY_DATA_SIZE_PADDING                       1
#define BHY_DATA_SIZE_QUATERNION                    11
#define BHY_DATA_SIZE_VECTOR                        8
#define BHY_DATA_SIZE_SCALAR_U8                     2
#define BHY_DATA_SIZE_SCALAR_U16                    3
#define BHY_DATA_SIZE_SCALAR_S16                    3
#define BHY_DATA_SIZE_SCALAR_U24                    4
#define BHY_DATA_SIZE_SENSOR_EVENT                  1
#define BHY_DATA_SIZE_UNCALIB_VECTOR                14
#define BHY_DATA_SIZE_META_EVENT                    4
#define BHY_DATA_SIZE_BSX                           17
#define BHY_DATA_SIZE_DEBUG                         14

/* set default custom sensor packet size to 1, same as padding */
#define BHY_DATA_SIZE_CUS1                          1
#define BHY_DATA_SIZE_CUS2                          1
#define BHY_DATA_SIZE_CUS3                          1
#define BHY_DATA_SIZE_CUS4                          1
#define BHY_DATA_SIZE_CUS5                          1

#define BHY_CALLBACK_MODE                           0

#define BHY_PAGE_SYSTEM                             1
#define BHY_PARAM_SYSTEM_META_EVENT_CTRL            1
#define BHY_PARAM_SYSTEM_FIFO_CTRL                  2
#define BHY_PARAM_SYSTEM_STAUS_BANK_0               3
#define BHY_PARAM_SYSTEM_STAUS_BANK_1               4
#define BHY_PARAM_SYSTEM_STAUS_BANK_2               5
#define BHY_PARAM_SYSTEM_STAUS_BANK_3               6
#define BHY_PARAM_SYSTEM_CUSTOM_VERSION             24
#define BHY_PARAM_SYSTEM_WAKE_UP_META_EVENT_CTRL    29
#define BHY_PARAM_SYSTEM_HOST_IRQ_TIMESTAMP         30
#define BHY_PARAM_SYSTEM_PHYSICAL_SENSOR_STATUS     31
#define BHY_PARAM_SYSTEM_PHYSICAL_SENSOR_PRESENT    32
#define BHY_PARAM_SYSTEM_PHYSICAL_SENSOR_DETAIL_0   32
#define BHY_PARAM_SYSTEM_PHYSICAL_SENSOR_DETAIL_ACC 33

/**************************************************************/
/**\name    READ PARAMETER REQUEST      */
/**************************************************************/
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_0      (0x00)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_1      (0x01)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_2      (0x02)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_3      (0x03)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_4      (0x04)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_5      (0x05)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_6      (0x06)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_7      (0x07)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_8      (0x08)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_9      (0x09)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_10     (0x0A)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_11     (0x0B)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_12     (0x0C)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_13     (0x0D)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_14     (0x0E)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_15     (0x0F)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_16     (0x10)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_17     (0x11)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_18     (0x12)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_19     (0x13)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_20     (0x14)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_21     (0x15)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_22     (0x16)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_23     (0x17)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_24     (0x18)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_25     (0x19)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_30     (0x1E)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_31     (0x1F)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_33     (0x21)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_34     (0x22)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_35     (0x23)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_36     (0x24)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_37     (0x25)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_38     (0x26)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_39     (0x27)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_40     (0x28)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_41     (0x29)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_42     (0x2A)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_43     (0x2B)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_44     (0x2C)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_45     (0x2D)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_46     (0x2E)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_47     (0x2F)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_48     (0x30)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_49     (0x31)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_50     (0x32)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_51     (0x33)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_52     (0x34)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_53     (0x35)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_54     (0x36)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_55     (0x37)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_56     (0x38)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_57     (0x39)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_63     (0x3F)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_64     (0x40)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_65     (0x41)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_66     (0x42)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_67     (0x43)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_68     (0x44)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_73     (0x49)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_74     (0x4A)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_75     (0x4B)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_76     (0x4C)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_77     (0x4D)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_78     (0x4E)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_79     (0x4F)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_80     (0x50)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_81     (0x51)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_82     (0x52)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_83     (0x53)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_84     (0x54)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_86     (0x56)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_87     (0x57)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_88     (0x58)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_89     (0x59)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_95     (0x5F)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_97     (0x61)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_98     (0x62)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_99     (0x63)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_100    (0x64)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_101    (0x65)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_102    (0x66)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_103    (0x67)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_104    (0x68)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_105    (0x69)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_106    (0x6A)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_107    (0x6B)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_108    (0x6C)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_109    (0x6D)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_110    (0x6E)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_111    (0x6F)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_112    (0x70)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_113    (0x71)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_114    (0x72)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_115    (0x73)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_116    (0x74)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_117    (0x75)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_118    (0x76)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_119    (0x77)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_120    (0x78)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_121    (0x79)
#define BHY_PARAMETER_REQUEST_READ_PARAMETER_127    (0x7F)
/**************************************************************/
/**\name    WRITE PARAMETER REQUEST   */
/**************************************************************/
#define BHY_PARAMETER_REQUEST_WRITE_PARAMETER_0     (0x80)
#define BHY_PARAMETER_REQUEST_WRITE_PARAMETER_1     (0x81)
#define BHY_PARAMETER_REQUEST_WRITE_PARAMETER_2     (0x82)
#define BHY_PARAMETER_REQUEST_WRITE_PARAMETER_3     (0x83)
#define BHY_PARAMETER_REQUEST_WRITE_PARAMETER_4     (0x84)
#define BHY_PARAMETER_REQUEST_WRITE_PARAMETER_5     (0x85)
#define BHY_PARAMETER_REQUEST_WRITE_PARAMETER_6     (0x86)
#define BHY_PARAMETER_REQUEST_WRITE_PARAMETER_7     (0x87)
#define BHY_PARAMETER_REQUEST_WRITE_PARAMETER_8     (0x88)
#define BHY_PARAMETER_REQUEST_WRITE_PARAMETER_9     (0x89)
#define BHY_PARAMETER_REQUEST_WRITE_PARAMETER_10    (0x8A)
#define BHY_PARAMETER_REQUEST_WRITE_PARAMETER_11    (0x8B)
/* non wakeup sensor configuration*/
#define BHY_PARAMETER_REQUEST_WRITE_PARAMETER_64    (0xC0)
#define BHY_PARAMETER_REQUEST_WRITE_PARAMETER_65    (0xC1)
#define BHY_PARAMETER_REQUEST_WRITE_PARAMETER_66    (0xC2)
#define BHY_PARAMETER_REQUEST_WRITE_PARAMETER_67    (0xC3)
#define BHY_PARAMETER_REQUEST_WRITE_PARAMETER_68    (0xC4)
#define BHY_PARAMETER_REQUEST_WRITE_PARAMETER_69    (0xC5)
#define BHY_PARAMETER_REQUEST_WRITE_PARAMETER_70    (0xC6)
#define BHY_PARAMETER_REQUEST_WRITE_PARAMETER_71    (0xC7)
#define BHY_PARAMETER_REQUEST_WRITE_PARAMETER_72    (0xC8)
#define BHY_PARAMETER_REQUEST_WRITE_PARAMETER_73    (0xC9)
#define BHY_PARAMETER_REQUEST_WRITE_PARAMETER_74    (0xCA)
#define BHY_PARAMETER_REQUEST_WRITE_PARAMETER_75    (0xCB)
#define BHY_PARAMETER_REQUEST_WRITE_PARAMETER_76    (0xCC)
#define BHY_PARAMETER_REQUEST_WRITE_PARAMETER_77    (0xCD)
#define BHY_PARAMETER_REQUEST_WRITE_PARAMETER_78    (0xCE)
#define BHY_PARAMETER_REQUEST_WRITE_PARAMETER_79    (0xCF)
#define BHY_PARAMETER_REQUEST_WRITE_PARAMETER_80    (0xD0)
#define BHY_PARAMETER_REQUEST_WRITE_PARAMETER_81    (0xD1)
#define BHY_PARAMETER_REQUEST_WRITE_PARAMETER_82    (0xD2)
#define BHY_PARAMETER_REQUEST_WRITE_PARAMETER_83    (0xD3)
#define BHY_PARAMETER_REQUEST_WRITE_PARAMETER_84    (0xD4)
#define BHY_PARAMETER_REQUEST_WRITE_PARAMETER_85    (0xD5)
#define BHY_PARAMETER_REQUEST_WRITE_PARAMETER_86    (0xD6)
#define BHY_PARAMETER_REQUEST_WRITE_PARAMETER_87    (0xD7)
#define BHY_PARAMETER_REQUEST_WRITE_PARAMETER_88    (0xD8)
#define BHY_PARAMETER_REQUEST_WRITE_PARAMETER_89    (0xD9)
#define BHY_PARAMETER_REQUEST_WRITE_PARAMETER_90    (0xDA)
#define BHY_PARAMETER_REQUEST_WRITE_PARAMETER_95    (0xDF)
/* wakeup sensor configuration*/
#define BHY_PARAMETER_REQUEST_WRITE_PARAMETER_97    (0xE1)
#define BHY_PARAMETER_REQUEST_WRITE_PARAMETER_98    (0xE2)
#define BHY_PARAMETER_REQUEST_WRITE_PARAMETER_99    (0xE3)
#define BHY_PARAMETER_REQUEST_WRITE_PARAMETER_100   (0xE4)
#define BHY_PARAMETER_REQUEST_WRITE_PARAMETER_101   (0xE5)
#define BHY_PARAMETER_REQUEST_WRITE_PARAMETER_102   (0xE6)
#define BHY_PARAMETER_REQUEST_WRITE_PARAMETER_103   (0xE7)
#define BHY_PARAMETER_REQUEST_WRITE_PARAMETER_104   (0xE8)
#define BHY_PARAMETER_REQUEST_WRITE_PARAMETER_105   (0xE9)
#define BHY_PARAMETER_REQUEST_WRITE_PARAMETER_106   (0xEA)
#define BHY_PARAMETER_REQUEST_WRITE_PARAMETER_107   (0xEB)
#define BHY_PARAMETER_REQUEST_WRITE_PARAMETER_108   (0xEC)
#define BHY_PARAMETER_REQUEST_WRITE_PARAMETER_109   (0xED)
#define BHY_PARAMETER_REQUEST_WRITE_PARAMETER_110   (0xEE)
#define BHY_PARAMETER_REQUEST_WRITE_PARAMETER_111   (0xEF)
#define BHY_PARAMETER_REQUEST_WRITE_PARAMETER_112   (0xF0)
#define BHY_PARAMETER_REQUEST_WRITE_PARAMETER_113   (0xF1)
#define BHY_PARAMETER_REQUEST_WRITE_PARAMETER_114   (0xF2)
#define BHY_PARAMETER_REQUEST_WRITE_PARAMETER_115   (0xF3)
#define BHY_PARAMETER_REQUEST_WRITE_PARAMETER_116   (0xF4)
#define BHY_PARAMETER_REQUEST_WRITE_PARAMETER_117   (0xF5)
#define BHY_PARAMETER_REQUEST_WRITE_PARAMETER_118   (0xF6)
#define BHY_PARAMETER_REQUEST_WRITE_PARAMETER_119   (0xF7)
#define BHY_PARAMETER_REQUEST_WRITE_PARAMETER_120   (0xF8)
#define BHY_PARAMETER_REQUEST_WRITE_PARAMETER_121   (0xF9)
#define BHY_PARAMETER_REQUEST_WRITE_PARAMETER_127   (0xFF)

#define BHY_GET_BITSLICE(regvar, bitname)           ((regvar & bitname##__MSK) >> bitname##__POS)

#define BHY_SET_BITSLICE(regvar, bitname, val)      ((regvar & ~bitname##__MSK) | ((val << bitname##__POS) & bitname##__MSK))

struct parameter_write_buffer_t
{
    uint8_t write_parameter_byte1; /**<parameter byte 1*/
    uint8_t write_parameter_byte2; /**<parameter byte 2*/
    uint8_t write_parameter_byte3; /**<parameter byte 3*/
    uint8_t write_parameter_byte4; /**<parameter byte 4*/
    uint8_t write_parameter_byte5; /**<parameter byte 5*/
    uint8_t write_parameter_byte6; /**<parameter byte 6*/
    uint8_t write_parameter_byte7; /**<parameter byte 7*/
    uint8_t write_parameter_byte8; /**<parameter byte 8*/
};

struct parameter_read_buffer_t
{
    uint8_t parameter_1;  /**<parameter bytes 1*/
    uint8_t parameter_2;  /**<parameter bytes 1*/
    uint8_t parameter_3;  /**<parameter bytes 1*/
    uint8_t parameter_4;  /**<parameter bytes 1*/
    uint8_t parameter_5;  /**<parameter bytes 1*/
    uint8_t parameter_6;  /**<parameter bytes 1*/
    uint8_t parameter_7;  /**<parameter bytes 1*/
    uint8_t parameter_8;  /**<parameter bytes 1*/
    uint8_t parameter_9;  /**<parameter bytes 1*/
    uint8_t parameter_10; /**<parameter bytes 1*/
    uint8_t parameter_11; /**<parameter bytes 1*/
    uint8_t parameter_12; /**<parameter bytes 1*/
    uint8_t parameter_13; /**<parameter bytes 1*/
    uint8_t parameter_14; /**<parameter bytes 1*/
    uint8_t parameter_15; /**<parameter bytes 1*/
    uint8_t parameter_16; /**<parameter bytes 1*/
};

struct accel_physical_status_t
{
    uint16_t accel_sample_rate;   /**<contains the accel sampling rate information*/
    uint16_t accel_dynamic_range; /**<contains the accel dynamic range information*/
    uint8_t  accel_flag;          /**<contains the accel flag information*/
};

struct gyro_physical_status_t
{
    uint16_t gyro_sample_rate;   /**<contains the gyro sampling rate information*/
    uint16_t gyro_dynamic_range; /**<contains the gyro dynamic range information*/
    uint8_t  gyro_flag;          /**<contains the gyro flag information*/
};

struct mag_physical_status_t
{
    uint16_t mag_sample_rate;   /**<contains the mag sampling rate information*/
    uint16_t mag_dynamic_range; /**<contains the mag dynamic range information*/
    uint8_t  mag_flag;          /**<contains the mag flag information*/
};

struct sensor_configuration_wakeup_t
{
    uint16_t wakeup_sample_rate;        /**<contains the sample rate information*/
    uint16_t wakeup_max_report_latency; /**<contains the maximum report latency*/
    uint16_t wakeup_change_sensitivity; /**<contains the sensitivity*/
    uint16_t wakeup_dynamic_range;      /**<contains the dynamic range*/
};

struct sensor_configuration_non_wakeup_t
{
    uint16_t non_wakeup_sample_rate;        /**<contains the sample rate information*/
    uint16_t non_wakeup_max_report_latency; /**<contains the maximum report latency*/
    uint16_t non_wakeup_change_sensitivity; /**<contains the sensitivity*/
    uint16_t non_wakeup_dynamic_range;      /**<contains the dynamic range*/
};

struct bhy_t
{
    uint8_t product_id;                       /**< product id of BHY */
    uint8_t device_addr;                      /**< device address of BHY */
    BHY_WR_FUNC_PTR;                          /**< bus write function pointer used to map the user bus
                                                 write functions*/
    BHY_RD_FUNC_PTR;                          /**< bus read function pointer used to map the user bus read
                                                 functions*/
    BHY_BRD_FUNC_PTR;                         /**< burst read function pointer used to map the user burst
                                                 read functions*/
    void (*delay_msec)(BHY_MDELAY_DATA_TYPE); /**< delay function pointer */
};

typedef enum
{
    BHY_META_EVENT_TYPE_NOT_USED = 0,
    BHY_META_EVENT_TYPE_FLUSH_COMPLETE = 1,
    BHY_META_EVENT_TYPE_SAMPLE_RATE_CHANGED = 2,
    BHY_META_EVENT_TYPE_POWER_MODE_CHANGED = 3,
    BHY_META_EVENT_TYPE_ERROR = 4,
    BHY_META_EVENT_TYPE_ALGORITHM = 5,
    /* IDs 6-10 are reserved */
    BHY_META_EVENT_TYPE_SENSOR_ERROR = 11,
    BHY_META_EVENT_TYPE_FIFO_OVERFLOW = 12,
    BHY_META_EVENT_TYPE_DYNAMIC_RANGE_CHANGED = 13,
    BHY_META_EVENT_TYPE_FIFO_WATERMARK = 14,
    BHY_META_EVENT_TYPE_SELF_TEST_RESULTS = 15,
    BHY_META_EVENT_TYPE_INITIALIZED = 16,

} bhy_meta_event_type_t;

typedef struct
{
    uint8_t sensor_id;
} bhy_data_padding_t;

typedef struct
{
    uint8_t sensor_id;
    int16_t x;
    int16_t y;
    int16_t z;
    int16_t w;
    int16_t estimated_accuracy;
} bhy_data_quaternion_t;

typedef struct
{
    uint8_t sensor_id;
    int16_t x;
    int16_t y;
    int16_t z;
    uint8_t status;
} bhy_data_vector_t;

typedef struct
{
    uint8_t sensor_id;
    uint8_t data;
} bhy_data_scalar_u8_t;

typedef struct
{
    uint8_t  sensor_id;
    uint16_t data;
} bhy_data_scalar_u16_t;

typedef struct
{
    uint8_t sensor_id;
    int16_t data;
} bhy_data_scalar_s16_t;

typedef struct
{
    uint8_t  sensor_id;
    uint32_t data;
} bhy_data_scalar_u24_t;

typedef struct
{
    uint8_t sensor_id;
} bhy_data_sensor_event_t;

typedef struct
{
    uint8_t sensor_id;
    int16_t x;
    int16_t y;
    int16_t z;
    int16_t x_bias;
    int16_t y_bias;
    int16_t z_bias;
    uint8_t status;
} bhy_data_uncalib_vector_t;

typedef struct
{
    uint8_t               meta_event_id;
    bhy_meta_event_type_t event_number;
    uint8_t               sensor_type;
    uint8_t               event_specific;
} bhy_data_meta_event_t;

typedef struct
{
    uint8_t  sensor_id;
    int32_t  x;
    int32_t  y;
    int32_t  z;
    uint32_t timestamp;
} bhy_data_bsx_t;

typedef struct
{
    uint8_t sensor_id;
    uint8_t data[13];
} bhy_data_debug_t;

typedef struct
{
    uint8_t  sensor_id;
    int16_t  deltaX;
    int16_t  deltaY;
    int16_t  deltaZ;
    int16_t  confidencelevel;
    uint16_t direction;
    uint16_t stepCount;
} bhy_data_pdr_t;

typedef struct
{
    uint8_t sensor_id;
    uint8_t data[16];
} bhy_data_custom_t;

typedef union
{
    bhy_data_padding_t        data_padding;
    bhy_data_quaternion_t     data_quaternion;
    bhy_data_vector_t         data_vector;
    bhy_data_scalar_u8_t      data_scalar_u8;
    bhy_data_scalar_u16_t     data_scalar_u16;
    bhy_data_scalar_s16_t     data_scalar_s16;
    bhy_data_scalar_u24_t     data_scalar_u24;
    bhy_data_sensor_event_t   data_sensor_event;
    bhy_data_uncalib_vector_t data_uncalib_vector;
    bhy_data_meta_event_t     data_meta_event;
    bhy_data_bsx_t            data_bsx;
    bhy_data_debug_t          data_debug;
    bhy_data_custom_t         data_custom;
    bhy_data_pdr_t            data_pdr;
} bhy_data_generic_t;

typedef enum
{
    BHY_DATA_TYPE_PADDING = 0,
    BHY_DATA_TYPE_QUATERNION = 1,
    BHY_DATA_TYPE_VECTOR = 2,
    BHY_DATA_TYPE_SCALAR_U8 = 3,
    BHY_DATA_TYPE_SCALAR_U16 = 4,
    BHY_DATA_TYPE_SCALAR_S16 = 5,
    BHY_DATA_TYPE_SCALAR_U24 = 6,
    BHY_DATA_TYPE_SENSOR_EVENT = 7,
    BHY_DATA_TYPE_UNCALIB_VECTOR = 8,
    BHY_DATA_TYPE_META_EVENT = 9,
    BHY_DATA_TYPE_BSX = 10,
    BHY_DATA_TYPE_DEBUG = 11,
    BHY_DATA_TYPE_CUS1 = 12,
    BHY_DATA_TYPE_CUS2 = 13,
    BHY_DATA_TYPE_CUS3 = 14,
    BHY_DATA_TYPE_CUS4 = 15,
    BHY_DATA_TYPE_CUS5 = 16,
} bhy_data_type_t;

typedef enum
{
    VS_TYPE_ACCELEROMETER = VS_ID_ACCELEROMETER,
    VS_TYPE_GEOMAGNETIC_FIELD = VS_ID_MAGNETOMETER,
    VS_TYPE_ORIENTATION = VS_ID_ORIENTATION,
    VS_TYPE_GYROSCOPE = VS_ID_GYROSCOPE,
    VS_TYPE_LIGHT = VS_ID_LIGHT,
    VS_TYPE_PRESSURE = VS_ID_BAROMETER,
    VS_TYPE_TEMPERATURE = VS_ID_TEMPERATURE,
    VS_TYPE_PROXIMITY = VS_ID_PROXIMITY,
    VS_TYPE_GRAVITY = VS_ID_GRAVITY,
    VS_TYPE_LINEAR_ACCELERATION = VS_ID_LINEAR_ACCELERATION,
    VS_TYPE_ROTATION_VECTOR = VS_ID_ROTATION_VECTOR,
    VS_TYPE_RELATIVE_HUMIDITY = VS_ID_HUMIDITY,
    VS_TYPE_AMBIENT_TEMPERATURE = VS_ID_AMBIENT_TEMPERATURE,
    VS_TYPE_MAGNETIC_FIELD_UNCALIBRATED = VS_ID_UNCALIBRATED_MAGNETOMETER,
    VS_TYPE_GAME_ROTATION_VECTOR = VS_ID_GAME_ROTATION_VECTOR,
    VS_TYPE_GYROSCOPE_UNCALIBRATED = VS_ID_UNCALIBRATED_GYROSCOPE,
    VS_TYPE_SIGNIFICANT_MOTION = VS_ID_SIGNIFICANT_MOTION,
    VS_TYPE_STEP_DETECTOR = VS_ID_STEP_DETECTOR,
    VS_TYPE_STEP_COUNTER = VS_ID_STEP_COUNTER,
    VS_TYPE_GEOMAGNETIC_ROTATION_VECTOR = VS_ID_GEOMAGNETIC_ROTATION_VECTOR,
    VS_TYPE_HEART_RATE = VS_ID_HEART_RATE,
    VS_TYPE_TILT = VS_ID_TILT_DETECTOR,
    VS_TYPE_WAKEUP = VS_ID_WAKE_GESTURE,
    VS_TYPE_GLANCE = VS_ID_GLANCE_GESTURE,
    VS_TYPE_PICKUP = VS_ID_PICKUP_GESTURE,
    VS_TYPE_CUS1 = VS_ID_CUS1,
    VS_TYPE_CUS2 = VS_ID_CUS2,
    VS_TYPE_CUS3 = VS_ID_CUS3,
    VS_TYPE_CUS4 = VS_ID_CUS4,
    VS_TYPE_CUS5 = VS_ID_CUS5,
    VS_TYPE_ACTIVITY_RECOGNITION = VS_ID_ACTIVITY
} bhy_virtual_sensor_t;

typedef enum
{
    PHYSICAL_SENSOR_INDEX_ACC = 0,
    PHYSICAL_SENSOR_INDEX_MAG,
    PHYSICAL_SENSOR_INDEX_GYRO,
    PHYSICAL_SENSOR_COUNT
} bhy_physical_sensor_index_type_t;

struct sensor_information_wakeup_t
{
    u8  wakeup_sensor_type;    /**<contains the sensor type*/
    u8  wakeup_driver_id;      /**<contains the driver id*/
    u8  wakeup_driver_version; /**<contains the driver version*/
    u8  wakeup_power;          /**<contains the power example 0.1mA*/
    u16 wakeup_max_range;
    /**<contains the maxim range of sensor data in SI units*/
    u16 wakeup_resolution;
    /**<contains the no of bit resolution of underlying sensor*/
    u16 wakeup_max_rate;      /**<contains the maximum rate in Hz*/
    u16 wakeup_fifo_reserved; /**< contains the fifo size*/
    u16 wakeup_fifo_max;      /**< contains the entire fifo size*/
    u8  wakeup_event_size;    /**< contains the no of bytes sensor data packet*/
    u8  wakeup_min_rate;      /**<contains the minimum rate in Hz*/
};

struct sensor_information_non_wakeup_t
{
    u8  non_wakeup_sensor_type;    /**<contains the sensor type*/
    u8  non_wakeup_driver_id;      /**<contains the driver id*/
    u8  non_wakeup_driver_version; /**<contains the driver version*/
    u8  non_wakeup_power;          /**<contains the power example 0.1mA*/
    u16 non_wakeup_max_range;
    /**<contains the maxim range of sensor data in SI units*/
    u16 non_wakeup_resolution;
    /**<contains the no of bit resolution of underlying sensor*/
    u16 non_wakeup_max_rate;      /**<contains the maximum rate in Hz*/
    u16 non_wakeup_fifo_reserved; /**< contains the fifo size*/
    u16 non_wakeup_fifo_max;      /**< contains the entire fifo size*/
    u8  non_wakeup_event_size;    /**< contains the no of bytes sensor data packet*/
    u8  non_wakeup_min_rate;      /**<contains the minimum rate in Hz*/
};

/* Cursor over a raw FIFO buffer, advanced in place by IMU_parse_next_raw_sample(). */
typedef struct
{
    const uint8_t *ptr;
    uint16_t       remaining;
} FifoCursor_t;

/* ── Public function declarations ────────────────────────────────────────── */

int8_t IMU_driver_init(const uint8_t *bhy_fw_data, mxc_i2c_regs_t *i2c_inst);

int8_t IMU_enable_virtual_sensor(mxc_i2c_regs_t      *i2c_inst,
                                 bhy_virtual_sensor_t sensor_id,
                                 uint8_t              wakeup_status,
                                 uint16_t             sample_rate,
                                 uint16_t             max_report_latency_ms,
                                 uint8_t              flush_sensor,
                                 uint16_t             change_sensitivity,
                                 uint16_t             dynamic_range);

int8_t bhy_mapping_matrix_set(mxc_i2c_regs_t *i2c_inst, bhy_physical_sensor_index_type_t index, int8_t *mapping_matrix);

int8_t bhy_mapping_matrix_get(mxc_i2c_regs_t *i2c_inst, bhy_physical_sensor_index_type_t index, int8_t *mapping_matrix);

int8_t IMU_read_bytes_remaining(mxc_i2c_regs_t *i2c_inst, uint16_t *bytes_remaining);

int8_t IMU_read_fifo(mxc_i2c_regs_t *i2c_inst, uint8_t *buffer, uint16_t size);

int8_t IMU_abort_fifo_transfer(mxc_i2c_regs_t *i2c_inst);

int8_t IMU_parse_next_raw_sample(FifoCursor_t       *cursor,
                                 bhy_data_generic_t *fifo_data_output,
                                 bhy_data_type_t    *fifo_data_type);

#endif /* IMU_H_ */
