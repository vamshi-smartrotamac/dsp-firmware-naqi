/**************************************************************************
 * Copyright (c) 2025 Naqi Logix Inc. All Rights Reserved.
 *
 * File: IMU.c
 *
 * This file is part of the proprietary firmware and is protected by
 * applicable copyright laws and international treaties. Unauthorized use,
 * reproduction, or distribution of this file or its contents is strictly
 * prohibited. For permissions, contact info @ naqilogix.com.
 **************************************************************************/

/*
 * BHI160.c
 *
 *  Created on: 21-Nov-2022
 *      Author: LENOVO
 */

#include "bhi160.h"
#include <i2c_driver.h>
#include <math.h>
#include <string.h>
#include "mxc_delay.h"
#include "logger.h"
#include "board_init.h"

#ifdef EVALBOARD
#define BHI160_DEV_ADDR (0x29)
#else
#define BHI160_DEV_ADDR (0x28)
#endif

// struct bhy_t p_bhy;
static struct bhy_t *p_bhy;

static const uint8_t _fifoSizes[] = {
    BHY_DATA_SIZE_PADDING,        BHY_DATA_SIZE_QUATERNION, BHY_DATA_SIZE_VECTOR,     BHY_DATA_SIZE_SCALAR_U8,
    BHY_DATA_SIZE_SCALAR_U16,     BHY_DATA_SIZE_SCALAR_S16, BHY_DATA_SIZE_SCALAR_U24, BHY_DATA_SIZE_SENSOR_EVENT,
    BHY_DATA_SIZE_UNCALIB_VECTOR, BHY_DATA_SIZE_META_EVENT, BHY_DATA_SIZE_BSX,        BHY_DATA_SIZE_DEBUG,
    BHY_DATA_SIZE_CUS1,           BHY_DATA_SIZE_CUS2,       BHY_DATA_SIZE_CUS3,       BHY_DATA_SIZE_CUS4,
    BHY_DATA_SIZE_CUS5,
};

struct parameter_read_buffer_t read_buffer;
struct parameter_write_buffer_t write_buffer;

static struct bhy_t bhy;


int8_t IMU_disable_virtual_sensor(mxc_i2c_regs_t *i2c_inst,
		bhy_virtual_sensor_t sensor_id, uint8_t wakeup_status);

int8_t bhy_mapping_matrix_get(mxc_i2c_regs_t *i2c_inst,
		bhy_physical_sensor_index_type_t index, int8_t *mapping_matrix);

int8_t IMU_Get_product_id(mxc_i2c_regs_t *i2c_inst, uint8_t *v_product_id_u8);

int8_t IMU_Set_Reset_Request(mxc_i2c_regs_t *i2c_inst,
		uint8_t v_reset_request_u8);

int8_t IMU_Initialize_support(mxc_i2c_regs_t *i2c_inst);

int8_t IMU_Write_Register(mxc_i2c_regs_t *i2c_inst, uint8_t v_addr_u8,
		uint8_t *v_data_u8, uint16_t v_len_u16);

int8_t IMU_get_crc_host(mxc_i2c_regs_t *i2c_inst, uint32_t *v_crc_host_u32);

int8_t IMU_get_rom_version(mxc_i2c_regs_t *i2c_inst,
		uint16_t *v_rom_version_u16);

int8_t IMU_initialize_from_rom(mxc_i2c_regs_t *i2c_inst, const uint8_t *memory,
		const uint32_t v_file_length_u32);

int8_t IMU_set_fifo_flush(mxc_i2c_regs_t *i2c_inst, uint8_t v_fifo_flush_u8);

int8_t IMU_write_parameter_bytes(mxc_i2c_regs_t *i2c_inst,
		uint8_t v_page_select_u8, uint8_t v_parameter_request_u8);

int8_t IMU_set_wakeup_sensor_configuration(mxc_i2c_regs_t *i2c_inst,
		struct sensor_configuration_wakeup_t sensor_configuration,
		uint8_t v_parameter_request_u8);

int8_t IMU_set_non_wakeup_sensor_configuration(mxc_i2c_regs_t *i2c_inst,
		struct sensor_configuration_non_wakeup_t sensor_configuration,
		uint8_t v_parameter_request_u8);

int8_t IMU_Set_parameter_request(mxc_i2c_regs_t *i2c_inst,
		int8_t v_parameter_request_u8);

int8_t IMU_set_parameter_page_select(mxc_i2c_regs_t *i2c_inst,
		uint8_t v_page_select_u8);

int8_t IMU_get_parameter_acknowledge(mxc_i2c_regs_t *i2c_inst,
		uint8_t *v_parameter_acknowledge_u8);

int8_t IMU_read_reg(mxc_i2c_regs_t *i2c_inst, uint8_t v_addr_u8,
		uint8_t *v_data_u8, uint16_t v_len_u16);

int8_t IMU_Init_device(mxc_i2c_regs_t *i2c_inst, struct bhy_t *bhy);

int8_t sensor_i2c_write(mxc_i2c_regs_t *i2c_inst, uint8_t addr, uint8_t reg,
		uint8_t *p_buf, uint16_t size);

int8_t sensor_i2c_read(mxc_i2c_regs_t *i2c_inst, uint8_t addr, uint8_t reg,
		uint8_t *p_buf, uint16_t size);

int8_t IMU_write_parameter_page(mxc_i2c_regs_t *i2c_inst, uint8_t page,
		uint8_t parameter, uint8_t *data, uint8_t length);

int8_t IMU_read_parameter_page(mxc_i2c_regs_t *i2c_inst, uint8_t page,
		uint8_t parameter, uint8_t *data, uint8_t length);

int8_t IMU_write_reg(mxc_i2c_regs_t *i2c_inst, uint8_t v_addr_u8,
		uint8_t *v_data_u8, uint16_t v_len_u16);

int8_t IMU_get_wakeup_sensor_information(mxc_i2c_regs_t *i2c_inst,
		uint8_t v_parameter_request_u8,
		struct sensor_information_wakeup_t *sensor_information);

int8_t IMU_get_non_wakeup_sensor_information(mxc_i2c_regs_t *i2c_inst,
		uint8_t v_parameter_request_u8,
		struct sensor_information_non_wakeup_t *sensor_information);

int8_t IMU_read_parameter_bytes(mxc_i2c_regs_t *i2c_inst,
                                uint8_t v_page_select_u8,
                                uint8_t v_parameter_request_u8);

void delay_ms(uint32_t delay);

void delay_ms(uint32_t delay)
{
    MXC_Delay(MXC_DELAY_MSEC(delay));
}

/*
 *---------------------------------------------------------------------------------------------
 * Function:      IMU_Get_product_id
 * Description:   This function reads the Product ID of device
 * r
 * Parameters:    [in1] = mxc_i2c_regs_t* i2c_inst    (i2C instance)
 * 				 [in2] = uint8_t *v_product_id_u8	 (to
 * store product ID)
 *
 * return         status of I2C transaction
 * ---------------------------------------------------------------------------------------------
 */
int8_t IMU_Get_product_id(mxc_i2c_regs_t *i2c_inst, uint8_t *v_product_id_u8)
{

    int8_t com_rslt = BHY_COMM_RES;
    uint8_t v_data_u8 = BHY_INIT_VALUE;

    /* check the p_bhy pointer as NULL*/
    if (BHY_NULL_PTR == p_bhy)
    {
        com_rslt = BHY_NULL;
    }
    else
    {

        /* read the load parameter request rate*/
        com_rslt = p_bhy->BHY_BUS_READ_FUNC(i2c_inst, p_bhy->device_addr, BHY_I2C_REG_PRODUCT_ID_ADDR, &v_data_u8,
                                            BHY_GEN_READ_WRITE_LENGTH);

        *v_product_id_u8 = v_data_u8;
    }
    return com_rslt;
}

/*
 *---------------------------------------------------------------------------------------------
 * Function:      IMU_Set_Reset_Request
 * Description:   This function request the reset
 * r
 * Parameters:    [in1] = mxc_i2c_regs_t* i2c_inst    (i2C instance)
 * 				 [in2] = uint8_t v_reset_request_u8	 (reset
 * enable)
 *
 * return         status of I2C transaction
 * ---------------------------------------------------------------------------------------------
 */
int8_t IMU_Set_Reset_Request(mxc_i2c_regs_t *i2c_inst, uint8_t v_reset_request_u8)
{
    /* variable used for return the status of communication result*/
    int8_t com_rslt = BHY_COMM_RES;
    u8 v_data_u8 = BHY_INIT_VALUE;

    /* check the p_bhy pointer as NULL*/
    if (BHY_NULL_PTR == p_bhy)
    {
        com_rslt = BHY_NULL;
    }
    else
    {
        v_data_u8 = v_reset_request_u8;

        /* write load parameter request*/
        com_rslt = p_bhy->BHY_BUS_WRITE_FUNC(i2c_inst, p_bhy->device_addr, BHY_I2C_REG_RESET_REQUEST__REG, &v_data_u8,
                                             BHY_GEN_READ_WRITE_LENGTH);
    }

    return com_rslt;
}

/*
 *---------------------------------------------------------------------------------------------
 * Function:      IMU_Init_device
 * Description:   This function Initializes the IMU sensor
 * r
 * Parameters:    [in1] = mxc_i2c_regs_t* i2c_inst (i2C instance)
 * 				 [in2] = bhy_t *bhy
 *  (bhy Structure)
 *
 * return         status of I2C transaction
 * ---------------------------------------------------------------------------------------------
 */
int8_t IMU_Init_device(mxc_i2c_regs_t *i2c_inst, struct bhy_t *bhy)
{

    int8_t com_rslt = BHY_COMM_RES;
    u8 v_data_u8 = BHY_INIT_VALUE;

    p_bhy = bhy;
    com_rslt = p_bhy->BHY_BUS_READ_FUNC(i2c_inst, p_bhy->device_addr, BHY_I2C_REG_PRODUCT_ID_ADDR, &v_data_u8,
                                        BHY_GEN_READ_WRITE_LENGTH);

    p_bhy->product_id = v_data_u8;
    return com_rslt;
}

/*
 *---------------------------------------------------------------------------------------------
 * Function:      IMU_Write_Register
 * Description:   This function writes the Bytes to the IMU Sensor
 * r
 * Parameters:    [in1] = mxc_i2c_regs_t* i2c_inst (i2C instance)
 * 				 [in2] = uint8_t  v_addr_u8        (8-bit
 * address) [in3] = uint8_t *v_data_u8        (8-bit data) [in4] = uint8_t
 * v_len_u16         (16-bit length)
 *
 * return         status of I2C transaction
 * ---------------------------------------------------------------------------------------------
 */

int8_t IMU_Write_Register(mxc_i2c_regs_t *i2c_inst, uint8_t v_addr_u8, uint8_t *v_data_u8, uint16_t v_len_u16)
{

    /* variable used for return the status of communication result*/
    int8_t com_rslt = BHY_COMM_RES;
    if (BHY_NULL_PTR == p_bhy)
    {
        com_rslt = BHY_NULL;
    }
    else
    {

        com_rslt = p_bhy->BHY_BUS_WRITE_FUNC(i2c_inst, p_bhy->device_addr, v_addr_u8, v_data_u8, v_len_u16);
    }
    return com_rslt;
}

/*
 *---------------------------------------------------------------------------------------------
 * Function:      IMU_read_reg
 * Description:   This function writes the Bytes to the IMU Sensor
 * r
 * Parameters:    [in1] = mxc_i2c_regs_t* i2c_inst (i2C instance)
 * 				 [in2] = uint8_t  v_addr_u8        (8-bit
 * address) [in3] = uint8_t *v_data_u8        (8-bit data) [in4] = uint8_t
 * v_len_u16         (16-bit length)
 *
 * return         status of I2C transaction
 * ---------------------------------------------------------------------------------------------
 */

int8_t IMU_read_reg(mxc_i2c_regs_t *i2c_inst, uint8_t v_addr_u8, uint8_t *v_data_u8, uint16_t v_len_u16)
{
    /* variable used for return the status of communication result*/
    int8_t com_rslt = BHY_COMM_RES;

    if (BHY_NULL_PTR == p_bhy)
    {
        com_rslt = BHY_NULL;
    }
    else
    {
        /* Read data from register*/
        com_rslt = p_bhy->BHY_BUS_READ_FUNC(i2c_inst, p_bhy->device_addr, v_addr_u8, v_data_u8, v_len_u16);
    }
    return com_rslt;
}

/*
 *---------------------------------------------------------------------------------------------
 * Function:      IMU_Initialize_support
 * Description:   This function sets the IMU i2C sensor driver read and write to
 * the custom firmware I2C driver and checks the device response Parameters:
 * [in1] = mxc_i2c_regs_t* i2c_inst (i2C instance) return         status of I2C
 * transaction
 * ---------------------------------------------------------------------------------------------
 */
int8_t IMU_Initialize_support(mxc_i2c_regs_t *i2c_inst)
{
    uint8_t tmp_retry = RETRY_NUM;
    bhy.bus_write = &sensor_i2c_write;
    bhy.bus_read = &sensor_i2c_read;
    bhy.delay_msec = (void *)&delay_ms;
    bhy.device_addr = BHI160_DEV_ADDR;

    IMU_Init_device(i2c_inst, &bhy);
    while (tmp_retry--)
    {
        NAQILOG_DEBUG("get product id return: %d", IMU_Get_product_id(i2c_inst, &bhy.product_id));
        if (bhy.product_id == 0x83)
        {
        	NAQILOG_INFO("Found device with ID 0x%x",bhy.product_id);
            return BHY_SUCCESS;
        }
    }

    return BHY_PRODUCT_ID_ERROR;
}

/*
 *---------------------------------------------------------------------------------------------
 * Function:      IMU_get_wakeup_sensor_information
 * Description:   This function reads the Wakeup sensor information and stores
 * in sensor information structure
 * r
 * Parameters:    [in1] = mxc_i2c_regs_t* i2c_inst
 *	(i2C instance) [in2] = uint8_t  v_parameter_request_u8        (8-bit)
 * 				 [in3] = sensor_information
 *		(Sensor Information structure) return         status of I2C
 * transaction
 * ---------------------------------------------------------------------------------------------
 */
int8_t IMU_get_wakeup_sensor_information(mxc_i2c_regs_t *i2c_inst, uint8_t v_parameter_request_u8,
                                         struct sensor_information_wakeup_t *sensor_information)
{
    /* variable used for return the status of communication result*/
    int8_t com_rslt = BHY_COMM_RES;

    /* input as page 3 and parameter request for sensor page*/
    com_rslt = IMU_read_parameter_bytes(i2c_inst, BHY_PAGE_3, v_parameter_request_u8);

    /* sensor type information */
    sensor_information->wakeup_sensor_type = (u8)(read_buffer.parameter_1);

    /* driver id information */
    sensor_information->wakeup_driver_id = (u8)(read_buffer.parameter_2);

    /* driver version information */
    sensor_information->wakeup_driver_version = (u8)(read_buffer.parameter_3);

    /* power information */
    sensor_information->wakeup_power = (u8)(read_buffer.parameter_4);

    /* maximum range information */
    sensor_information->wakeup_max_range =
        (u16)((read_buffer.parameter_6 << BHY_SHIFT_BIT_POSITION_BY_08_BITS) | (read_buffer.parameter_5));

    /* resolution information */
    sensor_information->wakeup_resolution =
        (u16)((read_buffer.parameter_8 << BHY_SHIFT_BIT_POSITION_BY_08_BITS) | (read_buffer.parameter_7));

    /* maximum rate information */
    sensor_information->wakeup_max_rate =
        (u16)((read_buffer.parameter_10 << BHY_SHIFT_BIT_POSITION_BY_08_BITS) | (read_buffer.parameter_9));

    /* fifo reserved information */
    sensor_information->wakeup_fifo_reserved =
        (u16)((read_buffer.parameter_12 << BHY_SHIFT_BIT_POSITION_BY_08_BITS) | (read_buffer.parameter_11));

    /* fifo max information */
    sensor_information->wakeup_fifo_max =
        (u16)((read_buffer.parameter_14 << BHY_SHIFT_BIT_POSITION_BY_08_BITS) | (read_buffer.parameter_13));

    /* event size information */
    sensor_information->wakeup_event_size = read_buffer.parameter_15;

    /* minimum rate information */
    sensor_information->wakeup_min_rate = read_buffer.parameter_16;

    return com_rslt;
}

/*
 *---------------------------------------------------------------------------------------------
 * Function:      IMU_get_non_wakeup_sensor_information
 * Description:   This function sets the IMU i2C sensor driver read and write to
 * the custom firmware I2C driver and checks the device response
 *
 * Parameters:    [in1] = mxc_i2c_regs_t* i2c_inst
 *	(i2C instance) [in2] = uint8_t  v_page_select_u8              (8-bit
 * Pageaddress) [in3] =uint8_t v_parameter_request_u8     		(8-bit
 * Parameter request) return         status of I2C transaction
 * ---------------------------------------------------------------------------------------------
 */
int8_t IMU_get_non_wakeup_sensor_information(mxc_i2c_regs_t *i2c_inst, uint8_t v_parameter_request_u8,
                                             struct sensor_information_non_wakeup_t *sensor_information)
{
    /* variable used for return the status of communication result*/
    int8_t com_rslt = BHY_COMM_RES;

    /* input as page 3 and parameter request for sensor page*/
    com_rslt = IMU_read_parameter_bytes(i2c_inst, BHY_PAGE_3, v_parameter_request_u8);

    /* sensor type information */
    sensor_information->non_wakeup_sensor_type = (u8)(read_buffer.parameter_1);

    /* driver id information */
    sensor_information->non_wakeup_driver_id = (u8)(read_buffer.parameter_2);

    /* driver version information */
    sensor_information->non_wakeup_driver_version = (u8)(read_buffer.parameter_3);

    /* power information */
    sensor_information->non_wakeup_power = (u8)(read_buffer.parameter_4);

    /* maximum range information */
    sensor_information->non_wakeup_max_range =
        (u16)((read_buffer.parameter_6 << BHY_SHIFT_BIT_POSITION_BY_08_BITS) | (read_buffer.parameter_5));

    /* resolution information */
    sensor_information->non_wakeup_resolution =
        (u16)((read_buffer.parameter_8 << BHY_SHIFT_BIT_POSITION_BY_08_BITS) | (read_buffer.parameter_7));

    /* maximum rate information */
    sensor_information->non_wakeup_max_rate =
        (u16)((read_buffer.parameter_10 << BHY_SHIFT_BIT_POSITION_BY_08_BITS) | (read_buffer.parameter_9));

    /* fifo reserved information */
    sensor_information->non_wakeup_fifo_reserved =
        (u16)((read_buffer.parameter_12 << BHY_SHIFT_BIT_POSITION_BY_08_BITS) | (read_buffer.parameter_11));

    /* fifo max information */
    sensor_information->non_wakeup_fifo_max =
        (u16)((read_buffer.parameter_14 << BHY_SHIFT_BIT_POSITION_BY_08_BITS) | (read_buffer.parameter_13));

    /* event size information */
    sensor_information->non_wakeup_event_size = read_buffer.parameter_15;

    /* minimum rate information */
    sensor_information->non_wakeup_min_rate = read_buffer.parameter_16;

    return com_rslt;
}

/*
 *---------------------------------------------------------------------------------------------
 * Function:      IMU_read_parameter_bytes
 * Description:   This function reads the IMU parameters and stores in read
 * buffer
 *
 * Parameters:    [in1] = mxc_i2c_regs_t* i2c_inst
 *	(i2C instance) [in2] = uint8_t  v_page_select_u8              (8-bit
 * Pageaddress) [in3] =uint8_t v_parameter_request_u8     		(8-bit
 * Parameter request) return         status of I2C transaction
 * ---------------------------------------------------------------------------------------------
 */
int8_t IMU_read_parameter_bytes(mxc_i2c_regs_t *i2c_inst, uint8_t v_page_select_u8, uint8_t v_parameter_request_u8)
{
    int8_t com_rslt = BHY_COMM_RES;
    u8 v_parameter_ack_u8 = BHY_INIT_VALUE;
    u8 init_array_data = BHY_INIT_VALUE;
    u8 a_read_data_u8[BHY_READ_BUFFER_SIZE];
    u8 v_parameter_ack_check_u8 = BHY_INIT_VALUE;

    for (; init_array_data < BHY_READ_BUFFER_SIZE; init_array_data++)
        a_read_data_u8[init_array_data] = BHY_INIT_VALUE;
    /* select the page*/
    com_rslt = IMU_set_parameter_page_select(i2c_inst, v_page_select_u8);
    /* select the parameter*/
    com_rslt += IMU_Set_parameter_request(i2c_inst, v_parameter_request_u8);
    /* read the values*/
    for (v_parameter_ack_check_u8 = BHY_INIT_VALUE; v_parameter_ack_check_u8 < BHY_PARAMETER_ACK_LENGTH;
         v_parameter_ack_check_u8++)
    {
        /* read Acknowledgement */
        com_rslt = IMU_get_parameter_acknowledge(i2c_inst, &v_parameter_ack_u8);
        if (v_parameter_ack_u8 == v_parameter_request_u8)
        {
            break;
        }
        else if (v_parameter_ack_u8 == BHY_PARAMETER_ACK_CHECK)
        {
            com_rslt = BHY_ERROR;
        }
    }
    com_rslt = p_bhy->BHY_BUS_READ_FUNC(i2c_inst, p_bhy->device_addr, BHY_I2C_REG_PARAMETER_READ_BUFFER_ZERO,
                                        a_read_data_u8, BHY_READ_BUFFER_LENGTH);
    read_buffer.parameter_1 = a_read_data_u8[BHY_READ_BUFFER_1_REG];
    read_buffer.parameter_2 = a_read_data_u8[BHY_READ_BUFFER_2_REG];
    read_buffer.parameter_3 = a_read_data_u8[BHY_READ_BUFFER_3_REG];
    read_buffer.parameter_4 = a_read_data_u8[BHY_READ_BUFFER_4_REG];
    read_buffer.parameter_5 = a_read_data_u8[BHY_READ_BUFFER_5_REG];
    read_buffer.parameter_6 = a_read_data_u8[BHY_READ_BUFFER_6_REG];
    read_buffer.parameter_7 = a_read_data_u8[BHY_READ_BUFFER_7_REG];
    read_buffer.parameter_8 = a_read_data_u8[BHY_READ_BUFFER_8_REG];
    read_buffer.parameter_9 = a_read_data_u8[BHY_READ_BUFFER_9_REG];
    read_buffer.parameter_10 = a_read_data_u8[BHY_READ_BUFFER_10_REG];
    read_buffer.parameter_11 = a_read_data_u8[BHY_READ_BUFFER_11_REG];
    read_buffer.parameter_12 = a_read_data_u8[BHY_READ_BUFFER_12_REG];
    read_buffer.parameter_13 = a_read_data_u8[BHY_READ_BUFFER_13_REG];
    read_buffer.parameter_14 = a_read_data_u8[BHY_READ_BUFFER_14_REG];
    read_buffer.parameter_15 = a_read_data_u8[BHY_READ_BUFFER_15_REG];
    read_buffer.parameter_16 = a_read_data_u8[BHY_READ_BUFFER_16_REG];

    /* end the parameter transfer (datasheet Config Parameter I/O procedure) */
    com_rslt += IMU_set_parameter_page_select(i2c_inst, 0);

    return com_rslt;
}

/*
 *---------------------------------------------------------------------------------------------
 * Function:      IMU_driver_init
 * Description:   This function is used to Initialize the Driver by loading the
 * Firmware array to sensor
 *
 * Parameters:     [in1] = uint8_t *bhy_fw_data
 *	(Firmware Array) [in2] = mxc_i2c_regs_t* i2c_inst
 *	(i2C instance)
 *
 * return          IMU Transaction status
 * ---------------------------------------------------------------------------------------------
 */

int8_t IMU_driver_init(const uint8_t *bhy_fw_data, mxc_i2c_regs_t *i2c_inst)
{

    uint32_t tmp_fw_len = 0;
    int8_t init_retry_count = 3;
    int8_t result = BHY_SUCCESS;

    /* get Firmware length */
    tmp_fw_len = 16 + bhy_fw_data[12] + (256 * bhy_fw_data[13]);


    result = IMU_Initialize_support(i2c_inst);

    IMU_Set_Reset_Request(i2c_inst, BHY_RESET_ENABLE);
    /* retry BHY_INIT_RETRY_COUNT times to avoid firmware download fail*/
    while (init_retry_count > 0)
    {
        /* downloads the ram patch to the BHI160 */
        result = IMU_initialize_from_rom(i2c_inst, bhy_fw_data, tmp_fw_len);
        if (result == BHY_SUCCESS)
        {
        	NAQILOG_INFO("IMU firmware upload Successful");
            break;
        }
        else{
        	NAQILOG_ERROR("IMU firmware upload failed, retry count %d",init_retry_count);
        }

        init_retry_count--;
    }

    return result;
}
/*!
 * @brief                This function set mapping matrix to a corresponding
 * physical sensor.
 *
 * @param[in]            index                physical sensor index
 * @param[in]            mapping_matrix       pointer to a int8_t
 * mapping_matrix[9]
 *
 * @retval               result of execution
 */
int8_t bhy_mapping_matrix_set(mxc_i2c_regs_t *i2c_inst, bhy_physical_sensor_index_type_t index, int8_t *mapping_matrix)
{
    uint8_t data[8] = {
        0,
    };
    int32_t i;
    int32_t handle;
    int8_t ret = BHY_SUCCESS;

    switch (index)
    {
    case PHYSICAL_SENSOR_INDEX_ACC:
        handle = VS_ID_ACCELEROMETER;
        break;
    case PHYSICAL_SENSOR_INDEX_MAG:
        handle = VS_ID_UNCALIBRATED_MAGNETOMETER;
        break;
    case PHYSICAL_SENSOR_INDEX_GYRO:
        handle = VS_ID_UNCALIBRATED_GYROSCOPE;
        break;
    default:
        return BHY_ERROR;
    }

    for (i = 0; i < 5; ++i)
    {
        switch (mapping_matrix[2 * i])
        {
        case 0:
            data[i] = 0;
            break;
        case 1:
            data[i] = 1;
            break;
        case -1:
            data[i] = 0xF;
            break;
        default:
            return BHY_ERROR;
        }

        if (i == 4)
        {
            break;
        }

        switch (mapping_matrix[2 * i + 1])
        {
        case 0:
            break;
        case 1:
            data[i] |= 0x10;
            break;
        case -1:
            data[i] |= 0xF0;
            break;
        default:
            return BHY_ERROR;
        }
    }

    ret = IMU_write_parameter_page(i2c_inst, BHY_PAGE_SYSTEM, BHY_PARAM_SYSTEM_PHYSICAL_SENSOR_DETAIL_0 + handle, data,
                                   sizeof(data));

        return ret;

}

/*!
 * @brief                This function get mapping matrix from a corresponding
 * physical sensor.
 *
 * @param[in]            index                physical sensor index
 * @param[in]            mapping_matrix       pointer to a int8_t
 * mapping_matrix[9]
 *
 * @retval               result of execution
 */
int8_t bhy_mapping_matrix_get(mxc_i2c_regs_t *i2c_inst, bhy_physical_sensor_index_type_t index, int8_t *mapping_matrix)
{
    int32_t i, j;
    int8_t ret = BHY_SUCCESS;
    uint8_t data[16];
    uint8_t map[32];
    uint8_t handle[3] = {
        VS_ID_ACCELEROMETER,
        VS_ID_UNCALIBRATED_MAGNETOMETER,
        VS_ID_UNCALIBRATED_GYROSCOPE,
    };
    uint8_t param;

    /* Check sensor existance */
    ret = IMU_read_parameter_page(i2c_inst, BHY_PAGE_SYSTEM, BHY_PARAM_SYSTEM_PHYSICAL_SENSOR_PRESENT, data, sizeof(data));
    if (ret < 0)
    {
        return ret;
    }

    for (i = 0; i < 4; ++i)
    {
        for (j = 0; j < 8; ++j)
        {
            if (data[i] & (1 << j))
            {
                map[i * 8 + j] = 1;
            }
            else
            {
                map[i * 8 + j] = 0;
            }
        }
    }

    if (!map[handle[index]])
    {
        return BHY_ERROR;
    }
    param = BHY_PARAM_SYSTEM_PHYSICAL_SENSOR_DETAIL_0 + handle[index];
    ret = IMU_read_parameter_page(i2c_inst, BHY_PAGE_SYSTEM, param, data, sizeof(data));
    if (ret < 0)
    {
        return ret;
    }

    mapping_matrix[0] = ((data[11] & 0x0F) == 0x0F) ? (-1) : (data[11] & 0x0F);
    mapping_matrix[1] = (((data[11] >> 4) & 0x0F) == 0x0F) ? (-1) : ((data[11] >> 4) & 0x0F);
    mapping_matrix[2] = (data[12] & 0x0F) == 0x0F ? (-1) : (data[12] & 0x0F);
    mapping_matrix[3] = (((data[12] >> 4) & 0x0F) == 0x0F) ? (-1) : ((data[12] >> 4) & 0x0F);
    mapping_matrix[4] = (data[13] & 0x0F) == 0x0F ? (-1) : (data[13] & 0x0F);
    mapping_matrix[5] = (((data[13] >> 4) & 0x0F) == 0x0F) ? (-1) : ((data[13] >> 4) & 0x0F);
    mapping_matrix[6] = (data[14] & 0x0F) == 0x0F ? (-1) : (data[14] & 0x0F);
    mapping_matrix[7] = (((data[14] >> 4) & 0x0F) == 0x0F) ? (-1) : ((data[14] >> 4) & 0x0F);
    mapping_matrix[8] = (data[15] & 0x0F) == 0x0F ? (-1) : (data[15] & 0x0F);

    return ret;
}

/*
 *---------------------------------------------------------------------------------------------
 * Function:      IMU_initialize_from_rom
 * Description:   This function is used to Download the Ram patch into the
 * BHI160 sensor
 *
 * Parameters:     [in1] = mxc_i2c_regs_t* i2c_inst 			(i2C
 * instance) [in2] = uint8_t *memory 				    (Firmware
 * Array) [in3] = uint32_t v_file_length_u32 		(File length)
 *
 * return          IMU Transaction status
 * ---------------------------------------------------------------------------------------------
 */

int8_t IMU_initialize_from_rom(mxc_i2c_regs_t *i2c_inst, const uint8_t *memory, const uint32_t v_file_length_u32)
{
    /* variable used for return the status of communication result*/
    int8_t com_rslt = BHY_COMM_RES;
    uint8_t v_upload_addr = BHY_UPLOAD_DATA;
    uint8_t v_chip_control_u8 = BHY_CHIP_CTRL_ENABLE_1;
    uint32_t v_crc_from_memory_u32 = BHY_INIT_VALUE;
    uint32_t v_crc_host_u32 = BHY_INIT_VALUE;
    uint32_t write_data = BHY_INIT_VALUE;
    uint8_t data_from_mem[BHY_SIGNATURE_MEM_LEN];
    uint8_t data_byte[BHY_RAM_WRITE_LENGTH_API];
    uint32_t read_index_u8 = BHY_INIT_VALUE;
    uint32_t reverse_index_u32 = BHY_INIT_VALUE;
    uint32_t reverse_block_index_u32 = BHY_INIT_VALUE;
    uint32_t write_length = BHY_INIT_VALUE;
    uint32_t data_to_process = BHY_INIT_VALUE;
    uint32_t packet_length = BHY_INIT_VALUE;
    uint16_t signature_flag = 0;
    uint16_t rom_version = 0;
    uint8_t rom_ver_exp = 0;
    uint8_t i = BHY_INIT_VALUE;

    /* initialize the array*/
    for (i = BHY_INIT_VALUE; i < BHY_SIGNATURE_MEM_LEN; i++)
    {
        data_from_mem[i] = BHY_INIT_VALUE;
    }
    for (i = BHY_INIT_VALUE; i < BHY_RAM_WRITE_LENGTH; i++)
    {
        data_byte[i] = BHY_INIT_VALUE;
    }

    if (BHY_NULL_PTR == p_bhy)
    {
        com_rslt = BHY_NULL;
    }
    else
    {

        /* Assign the memory data into the local array*/
        for (read_index_u8 = BHY_INIT_VALUE; read_index_u8 <= BHY_SIGNATURE_LENGTH; read_index_u8++)
        {
            data_from_mem[read_index_u8] = *(memory + read_index_u8);
        }

        /* Verify the signature of the data*/
        if ((data_from_mem[BHY_SIGNATURE_1] == BHY_IMAGE_SIGNATURE1) &&
            (data_from_mem[BHY_SIGNATURE_2] == BHY_IMAGE_SIGNATURE2))
        {
            com_rslt = BHY_SUCCESS;
        }
        else
        {
            com_rslt = BHY_ERROR;
            goto bhy_init_from_rom_return;
        }

        /* Verify the signature of the data*/
        signature_flag = data_from_mem[BHY_SIG_FLAG_1_POS] + ((uint16_t)data_from_mem[BHY_SIG_FLAG_2_POS] << 8);

        rom_ver_exp = BHY_GET_ROMVEREXP(signature_flag);
        NAQILOG_INFO("ROM Signature expected : %d",rom_ver_exp);

        IMU_get_rom_version(i2c_inst, &rom_version);
        NAQILOG_INFO("ROM version from IMU : 0x%x",rom_version);

        if (BHY_ROM_VER_DI01 == rom_ver_exp)
        {
            if (BHY_ROM_VERSION_DI01 == rom_version)
            {
                com_rslt = BHY_SUCCESS;
            }
            else
            {
                com_rslt = BHY_RAMPATCH_NOT_MATCH;
                goto bhy_init_from_rom_return;
            }
        }
        else if (BHY_ROM_VER_DI03 == rom_ver_exp)
        {
            if (BHY_ROM_VERSION_DI03 == rom_version)
            {
                com_rslt = BHY_SUCCESS;
            }
            else
            {
                com_rslt = BHY_RAMPATCH_NOT_MATCH;
                goto bhy_init_from_rom_return;
            }
        }
        else
        {
            com_rslt = BHY_RAMPATCH_NOT_SUPPORT;
            goto bhy_init_from_rom_return;
        }
        /* read the CRC data from memory */
        v_crc_from_memory_u32 =
            (uint32_t)(((uint32_t)data_from_mem[BHY_CRC_HOST_FILE_MSB] << BHY_SHIFT_BIT_POSITION_BY_24_BITS) |
                       ((uint32_t)data_from_mem[BHY_CRC_HOST_FILE_XXLSB] << BHY_SHIFT_BIT_POSITION_BY_16_BITS) |
                       (data_from_mem[BHY_CRC_HOST_FILE_XLSB] << BHY_SHIFT_BIT_POSITION_BY_08_BITS) |
                       (data_from_mem[BHY_CRC_HOST_FILE_LSB]));
        /* Remove the first 16 bytes*/
        data_to_process = v_file_length_u32 - BHY_SIGNATURE_LENGTH;

        /* set the reset as 0x01*/

        com_rslt = IMU_Set_Reset_Request(i2c_inst, BHY_RESET_ENABLE);

        com_rslt =
            IMU_Write_Register(i2c_inst, BHY_I2C_REG_CHIP_CONTROL_ADDR, &v_chip_control_u8, BHY_GEN_READ_WRITE_LENGTH);
        if (com_rslt != BHY_SUCCESS){
            goto bhy_init_from_rom_return;}
        /* set the upload data*/
        com_rslt = IMU_Write_Register(i2c_inst, BHY_I2C_REG_UPLOAD_0_ADDR, &v_upload_addr, BHY_GEN_READ_WRITE_LENGTH);
        if (com_rslt != BHY_SUCCESS){
            goto bhy_init_from_rom_return;}
        com_rslt = IMU_Write_Register(i2c_inst, BHY_I2C_REG_UPLOAD_1_ADDR, &v_upload_addr, BHY_GEN_READ_WRITE_LENGTH);
        if (com_rslt != BHY_SUCCESS){
            goto bhy_init_from_rom_return;}
        /* write the chip control register as 0x02*/
        write_length = data_to_process / BHY_RAM_WRITE_LENGTH_API;

        read_index_u8 = BHY_INIT_VALUE;

        /* write the memory of data */
        /*skips first 16 bytes*/
        write_data += 16;
        if (com_rslt == BHY_SUCCESS)
        {
            for (read_index_u8 = BHY_INIT_VALUE; read_index_u8 <= write_length; read_index_u8++)
            {
            	uint32_t remaining = data_to_process % BHY_RAM_WRITE_LENGTH_API;

            	if (read_index_u8 == write_length)
            	{
            	    if (remaining == 0)
            	        packet_length = BHY_RAM_WRITE_LENGTH_API / BHY_RAM_WRITE_LENGTH;
            	    else
            	        packet_length = (remaining + (BHY_RAM_WRITE_LENGTH - 1)) / BHY_RAM_WRITE_LENGTH;
            	}
            	else
            	{
            	    packet_length = BHY_RAM_WRITE_LENGTH_API / BHY_RAM_WRITE_LENGTH;
            	}
                /*reverse the data*/
                for (reverse_block_index_u32 = 1; reverse_block_index_u32 <= packet_length; reverse_block_index_u32++)
                {
                    for (reverse_index_u32 = 0; reverse_index_u32 < BHY_RAM_WRITE_LENGTH; reverse_index_u32++)
                    {
                        data_byte[reverse_index_u32 + ((reverse_block_index_u32 - 1) * BHY_RAM_WRITE_LENGTH)] =
                            *(memory + write_data + BHY_RAM_WRITE_LENGTH * reverse_block_index_u32 -
                              (reverse_index_u32 + 1));
                    }
                }

                if (packet_length != 0)
                    com_rslt = IMU_Write_Register(i2c_inst, BHY_I2C_REG_UPLOAD_DATA_ADDR, data_byte,
                                                   packet_length * BHY_RAM_WRITE_LENGTH);
                if (com_rslt != BHY_SUCCESS)
                    goto bhy_init_from_rom_return;
                write_data = write_data + (packet_length * BHY_RAM_WRITE_LENGTH);
            }
        }

        /* Check the CRC success*/
        com_rslt = IMU_get_crc_host(i2c_inst, &v_crc_host_u32);
        if (v_crc_from_memory_u32 == v_crc_host_u32)
        {
            com_rslt = BHY_SUCCESS;
        }
        else
        {
            com_rslt = BHY_CRC_ERROR;
            goto bhy_init_from_rom_return;
        }

        /* disable upload mode*/
        v_chip_control_u8 = BHY_CHIP_CTRL_ENABLE_2;
        /* write the chip control register as 0x02*/
        com_rslt =
            IMU_Write_Register(i2c_inst, BHY_I2C_REG_CHIP_CONTROL_ADDR, &v_chip_control_u8, BHY_GEN_READ_WRITE_LENGTH);
        if (com_rslt != BHY_SUCCESS){
            goto bhy_init_from_rom_return;}
    }
bhy_init_from_rom_return:
    return com_rslt;
}

/*
 *---------------------------------------------------------------------------------------------
 * Function:      IMU_get_crc_host
 * Description:   This function is reads the CRC of Firmware written
 *
 * Parameters:     [in1] = mxc_i2c_regs_t* i2c_inst 			(i2C
 * instance) [in2] = uint32_t *v_crc_host_u32			(CRC value )
 *
 *
 * return          IMU Transaction status
 * ---------------------------------------------------------------------------------------------
 */
int8_t IMU_get_crc_host(mxc_i2c_regs_t *i2c_inst, uint32_t *v_crc_host_u32)
{
    /* variable used for return the status of communication result*/
    int8_t com_rslt = BHY_COMM_RES;

    /* Array contains the sensor time it is 32 bit data
    a_data_u8[0] - crc HOST
    a_data_u8[1] - crc HOST
    a_data_u8[2] - crc HOST
    a_data_u8[3] - crc HOST
     */

    uint8_t a_data_u8[BHY_CRC_HOST_SIZE] = {BHY_INIT_VALUE, BHY_INIT_VALUE, BHY_INIT_VALUE, BHY_INIT_VALUE};
    // uint8 a_data_u8[BHY_CRC_HOST_SIZE] = {0};

    /* check the p_bhy pointer as NULL*/
    if (BHY_NULL_PTR == p_bhy)
    {
        com_rslt = BHY_NULL;
    }
    else
    {
        com_rslt = p_bhy->BHY_BUS_READ_FUNC(i2c_inst, p_bhy->device_addr, BHY_I2C_REG_CRC_HOST_ADDR, a_data_u8,
                                            BHY_CRC_HOST_LENGTH);
        *v_crc_host_u32 =
            (u32)(((u32)a_data_u8[BHY_CRC_HOST_MSB] << BHY_SHIFT_BIT_POSITION_BY_24_BITS) |
                  ((u32)a_data_u8[BHY_CRC_HOST_XXLSB] << BHY_SHIFT_BIT_POSITION_BY_16_BITS) |
                  (a_data_u8[BHY_CRC_HOST_XLSB] << BHY_SHIFT_BIT_POSITION_BY_08_BITS) | (a_data_u8[BHY_CRC_HOST_LSB]));
    }
    return com_rslt;
}

/*
 *---------------------------------------------------------------------------------------------
 * Function:      IMU_set_fifo_flush
 * Description:   This function is used to flush the fifo buffer
 *
 * Parameters:     [in1] = mxc_i2c_regs_t* i2c_inst 			(i2C
 * instance) [in2] = uint8_t v_fifo_flush_u8			(sensor ID to
 * flush)
 *
 *
 * return          IMU Transaction status
 * ---------------------------------------------------------------------------------------------
 */

int8_t IMU_set_fifo_flush(mxc_i2c_regs_t *i2c_inst, uint8_t v_fifo_flush_u8)
{
    /* variable used for return the status of communication result*/
    int8_t com_rslt = BHY_COMM_RES;
    uint8_t v_data_u8 = BHY_INIT_VALUE;

    if (BHY_NULL_PTR == p_bhy)
    {
        com_rslt = BHY_NULL;
    }
    else
    {
        com_rslt = p_bhy->BHY_BUS_READ_FUNC(i2c_inst, p_bhy->device_addr, BHY_I2C_REG_FIFO_FLUSH__REG, &v_data_u8,
                                            BHY_GEN_READ_WRITE_LENGTH);
        if (BHY_SUCCESS == com_rslt)
        {
            v_data_u8 = BHY_SET_BITSLICE(v_data_u8, BHY_I2C_REG_FIFO_FLUSH, v_fifo_flush_u8);
            com_rslt += p_bhy->BHY_BUS_WRITE_FUNC(i2c_inst, p_bhy->device_addr, BHY_I2C_REG_FIFO_FLUSH__REG, &v_data_u8,
                                                  BHY_GEN_READ_WRITE_LENGTH);
        }
    }
    return com_rslt;
}

/*
 *---------------------------------------------------------------------------------------------
 * Function:      IMU_get_rom_version
 * Description:   This function is used get the ROM Version loaded
 *
 * Parameters:     [in1] = mxc_i2c_regs_t* i2c_inst 			(i2C
 * instance) [in2] = uint16_t *v_rom_version_u16		(to store ROM
 * Version)
 *
 *
 * return          IMU Transaction status
 * ---------------------------------------------------------------------------------------------
 */

int8_t IMU_get_rom_version(mxc_i2c_regs_t *i2c_inst, uint16_t *v_rom_version_u16)
{
    /* variable used for return the status of communication result*/
    int8_t com_rslt = BHY_COMM_RES;
    uint8_t v_data_u8[BHY_ROM_VERSION_SIZE] = {BHY_INIT_VALUE, BHY_INIT_VALUE};

    /* check the p_bhy pointer as NULL*/
    if (BHY_NULL_PTR == p_bhy)
    {
        com_rslt = BHY_NULL;
    }
    else
    {

        /* read the load parameter request rate*/
        com_rslt = p_bhy->BHY_BUS_READ_FUNC(i2c_inst, p_bhy->device_addr, BHY_ROM_VERSION_ADDR, v_data_u8,
                                            BHY_ROM_VERSION_SIZE);

        *v_rom_version_u16 = (u16)((v_data_u8[BHY_ROM_VERSION_MSB_DATA] << BHY_SHIFT_BIT_POSITION_BY_08_BITS) |
                                   (v_data_u8[BHY_ROM_VERSION_LSB_DATA]));
    }
    return com_rslt;
}

/*
 *---------------------------------------------------------------------------------------------
 * Function:      IMU_enable_virtual_sensor
 * Description:   This function is used enable the Virtual sensor of IMU
 *
 * Parameters:     [in1] = mxc_i2c_regs_t* i2c_inst 			(i2C
 * instance) [in2] = bhy_virtual_sensor_t sensor_id	(Sensor ID) [in3] =
 * uint8_t wakeup_status				(Wakeup status) [in4] =
 * uint16_t sample_rate				(Reading sample rate) [in5] =
 * uint16_t max_report_latency_ms	(Latency in milliseconds) [in6] =
 * uint8_t flush_sensor				(Sensor ID to flush the fifo)
 * 				  [in7] = uint16_t change_sensitivity
 *	(Change sensitivity) [in8] = uint16_t dynamic_range
 *	(Dynamic range value)
 *
 * return          IMU Transaction status
 * ---------------------------------------------------------------------------------------------
 */

int8_t IMU_enable_virtual_sensor(mxc_i2c_regs_t *i2c_inst, bhy_virtual_sensor_t sensor_id, uint8_t wakeup_status,
                                 uint16_t sample_rate, uint16_t max_report_latency_ms, uint8_t flush_sensor,
                                 uint16_t change_sensitivity, uint16_t dynamic_range)
{
    int8_t result = BHY_SUCCESS;
    union {
        struct sensor_configuration_wakeup_t sensor_configuration_wakeup;
        struct sensor_configuration_non_wakeup_t sensor_configuration_non_wakeup;
    } sensor_configuration;

    /* checks if sensor id is in range */
    if ((uint8_t)sensor_id >= MAX_SENSOR_ID)
    {
        return BHY_OUT_OF_RANGE;
    }

    /*computes the sensor id */
    sensor_id += wakeup_status;

    /* flush the fifo if requested */
    switch (flush_sensor)
    {
    case VS_FLUSH_SINGLE:
        result = IMU_set_fifo_flush(i2c_inst, sensor_id);
        break;
    case VS_FLUSH_ALL:
        result = IMU_set_fifo_flush(i2c_inst, VS_FLUSH_ALL);
        break;
    case VS_FLUSH_NONE:
        break;
    default:
        return BHY_OUT_OF_RANGE;
    }

    /* computes the param page as sensor_id + 0xC0 (sensor parameter write)*/
    sensor_id += SENSOR_PARAMETER_WRITE;

    /*calls the right function */
    switch (wakeup_status)
    {
    case VS_NON_WAKEUP:
        sensor_configuration.sensor_configuration_non_wakeup.non_wakeup_sample_rate = sample_rate;
        sensor_configuration.sensor_configuration_non_wakeup.non_wakeup_max_report_latency = max_report_latency_ms;
        sensor_configuration.sensor_configuration_non_wakeup.non_wakeup_change_sensitivity = change_sensitivity;
        sensor_configuration.sensor_configuration_non_wakeup.non_wakeup_dynamic_range = dynamic_range;
        result = IMU_set_non_wakeup_sensor_configuration(
            i2c_inst, sensor_configuration.sensor_configuration_non_wakeup, sensor_id);
        return result;
    case VS_WAKEUP:
        sensor_configuration.sensor_configuration_wakeup.wakeup_sample_rate = sample_rate;
        sensor_configuration.sensor_configuration_wakeup.wakeup_max_report_latency = max_report_latency_ms;
        sensor_configuration.sensor_configuration_wakeup.wakeup_change_sensitivity = change_sensitivity;
        sensor_configuration.sensor_configuration_wakeup.wakeup_dynamic_range = dynamic_range;
        result =
            IMU_set_wakeup_sensor_configuration(i2c_inst, sensor_configuration.sensor_configuration_wakeup, sensor_id);
        return result;
    default:
        return BHY_OUT_OF_RANGE;
    }
}

/*
 *---------------------------------------------------------------------------------------------
 * Function:      IMU_disable_virtual_sensor
 * Description:   This function is used enable the Virtual sensor of IMU
 *
 * Parameters:     [in1] = mxc_i2c_regs_t* i2c_inst 			(i2C
 * instance) [in2] = bhy_virtual_sensor_t sensor_id	(Sensor ID) [in3] =
 * uint8_t wakeup_status				(Wakeup status) [in4] =
 * uint16_t sample_rate				(Reading sample rate) [in5] =
 * uint16_t max_report_latency_ms	(Latency in milliseconds) [in6] =
 * uint8_t flush_sensor				(Sensor ID to flush the fifo)
 * 				  [in7] = uint16_t change_sensitivity
 *	(Change sensitivity) [in8] = uint16_t dynamic_range
 *	(Dynamic range value)
 *
 * return          IMU Transaction status
 * ---------------------------------------------------------------------------------------------
 */

int8_t IMU_disable_virtual_sensor(mxc_i2c_regs_t *i2c_inst, bhy_virtual_sensor_t sensor_id, uint8_t wakeup_status)
{
    uint8_t result = BHY_SUCCESS;
    union {
        struct sensor_configuration_wakeup_t sensor_configuration_wakeup;
        struct sensor_configuration_non_wakeup_t sensor_configuration_non_wakeup;
    } sensor_configuration;

    /* checks if sensor id is in range */
    if ((uint8_t)sensor_id >= MAX_SENSOR_ID)
    {
        return BHY_OUT_OF_RANGE;
    }

    /*computes the sensor id */
    sensor_id += wakeup_status + SENSOR_PARAMETER_WRITE;

    /*calls the right function */
    switch (wakeup_status)
    {
    case VS_NON_WAKEUP:
        sensor_configuration.sensor_configuration_non_wakeup.non_wakeup_sample_rate = 0;
        sensor_configuration.sensor_configuration_non_wakeup.non_wakeup_max_report_latency = 0;
        sensor_configuration.sensor_configuration_non_wakeup.non_wakeup_change_sensitivity = 0;
        sensor_configuration.sensor_configuration_non_wakeup.non_wakeup_dynamic_range = 0;
        result += IMU_set_non_wakeup_sensor_configuration(
            i2c_inst, sensor_configuration.sensor_configuration_non_wakeup, sensor_id);
        return result;
    case VS_WAKEUP:
        sensor_configuration.sensor_configuration_wakeup.wakeup_sample_rate = 0;
        sensor_configuration.sensor_configuration_wakeup.wakeup_max_report_latency = 0;
        sensor_configuration.sensor_configuration_wakeup.wakeup_change_sensitivity = 0;
        sensor_configuration.sensor_configuration_wakeup.wakeup_dynamic_range = 0;
        result +=
            IMU_set_wakeup_sensor_configuration(i2c_inst, sensor_configuration.sensor_configuration_wakeup, sensor_id);
        return result;
    default:
        return BHY_OUT_OF_RANGE;
    }
}
/* ── little-endian byte readers, relative to a fifo cursor position ────────── */
static inline int16_t read_i16_le(const uint8_t *p)
{
    return (int16_t)(((uint16_t)p[0]) | ((uint16_t)p[1] << 8));
}

static inline uint16_t read_u16_le(const uint8_t *p)
{
    return (uint16_t)(((uint16_t)p[0]) | ((uint16_t)p[1] << 8));
}

static inline uint32_t read_u24_le(const uint8_t *p)
{
    return ((uint32_t)p[0]) | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16);
}

static inline uint32_t read_u32_le(const uint8_t *p)
{
    return ((uint32_t)p[0]) | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

/*
 *---------------------------------------------------------------------------------------------
 * Function:      IMU_parse_next_raw_sample
 * Description:   This function is used to parse the next raw sample from the fifo
 buffer as per the Sensor ID
 *
 * Parameters:     [in1] = FifoCursor_t *cursor           (FIFO buffer cursor,
 advanced past the parsed sample)
 * 				  [in2] = bhy_data_generic_t * fifo_data_output
 (FIFO Data Output) [in3] = bhy_data_type_t * fifo_data_type          (FIFO Data
 Type)
 *
 * return          IMU Transaction status
 * ---------------------------------------------------------------------------------------------
 */

int8_t IMU_parse_next_raw_sample(FifoCursor_t *cursor, bhy_data_generic_t *fifo_data_output,
                                 bhy_data_type_t *fifo_data_type)
{
    const uint8_t *p = cursor->ptr;
    uint16_t i = 0;

    if (cursor->remaining == 0)
    {
        /* there are no more bytes in the fifo buffer to read */
        return BHY_SUCCESS;
    }

    /* the first fifo byte should be a known virtual sensor ID */
    switch (*p)
    {
    case VS_ID_PADDING:
        (*fifo_data_type) = BHY_DATA_TYPE_PADDING;
        fifo_data_output->data_padding.sensor_id = *p;
        break;

    case VS_ID_ROTATION_VECTOR:
    case VS_ID_ROTATION_VECTOR_WAKEUP:
    case VS_ID_GAME_ROTATION_VECTOR:
    case VS_ID_GAME_ROTATION_VECTOR_WAKEUP:
    case VS_ID_GEOMAGNETIC_ROTATION_VECTOR:
    case VS_ID_GEOMAGNETIC_ROTATION_VECTOR_WAKEUP:
        if (cursor->remaining < _fifoSizes[BHY_DATA_TYPE_QUATERNION])
            return BHY_OUT_OF_RANGE;
        (*fifo_data_type) = BHY_DATA_TYPE_QUATERNION;
        fifo_data_output->data_quaternion.sensor_id = *p;
        fifo_data_output->data_quaternion.x = read_i16_le(&p[1]);
        fifo_data_output->data_quaternion.y = read_i16_le(&p[3]);
        fifo_data_output->data_quaternion.z = read_i16_le(&p[5]);
        fifo_data_output->data_quaternion.w = read_i16_le(&p[7]);
        fifo_data_output->data_quaternion.estimated_accuracy = read_i16_le(&p[9]);
        break;

    case VS_ID_ACCELEROMETER:
    case VS_ID_ACCELEROMETER_WAKEUP:
    case VS_ID_MAGNETOMETER:
    case VS_ID_MAGNETOMETER_WAKEUP:
    case VS_ID_ORIENTATION:
    case VS_ID_ORIENTATION_WAKEUP:
    case VS_ID_GYROSCOPE:
    case VS_ID_GYROSCOPE_WAKEUP:
    case VS_ID_GRAVITY:
    case VS_ID_GRAVITY_WAKEUP:
    case VS_ID_LINEAR_ACCELERATION:
    case VS_ID_LINEAR_ACCELERATION_WAKEUP:
        if (cursor->remaining < _fifoSizes[BHY_DATA_TYPE_VECTOR])
        {
            return BHY_OUT_OF_RANGE;
        }

        (*fifo_data_type) = BHY_DATA_TYPE_VECTOR;
        fifo_data_output->data_vector.sensor_id = *p;
        fifo_data_output->data_vector.x = read_i16_le(&p[1]);
        fifo_data_output->data_vector.y = read_i16_le(&p[3]);
        fifo_data_output->data_vector.z = read_i16_le(&p[5]);
        fifo_data_output->data_vector.status = p[7];
        break;

    case VS_ID_HEART_RATE:
    case VS_ID_HEART_RATE_WAKEUP:
        if (cursor->remaining < _fifoSizes[BHY_DATA_TYPE_SCALAR_U8])
        {
            return BHY_OUT_OF_RANGE;
        }

        (*fifo_data_type) = BHY_DATA_TYPE_SCALAR_U8;
        fifo_data_output->data_scalar_u8.sensor_id = *p;
        fifo_data_output->data_scalar_u8.data = p[1];
        break;

    case VS_ID_LIGHT:
    case VS_ID_LIGHT_WAKEUP:
    case VS_ID_PROXIMITY:
    case VS_ID_PROXIMITY_WAKEUP:
    case VS_ID_HUMIDITY:
    case VS_ID_HUMIDITY_WAKEUP:
    case VS_ID_STEP_COUNTER:
    case VS_ID_STEP_COUNTER_WAKEUP:
    case VS_ID_ACTIVITY:
    case VS_ID_ACTIVITY_WAKEUP:
    case VS_ID_TIMESTAMP_LSW:
    case VS_ID_TIMESTAMP_LSW_WAKEUP:
    case VS_ID_TIMESTAMP_MSW:
    case VS_ID_TIMESTAMP_MSW_WAKEUP:
        if (cursor->remaining < _fifoSizes[BHY_DATA_TYPE_SCALAR_U16])
        {
            return BHY_OUT_OF_RANGE;
        }

        (*fifo_data_type) = BHY_DATA_TYPE_SCALAR_U16;
        fifo_data_output->data_scalar_u16.sensor_id = *p;
        fifo_data_output->data_scalar_u16.data = read_u16_le(&p[1]);
        break;

    case VS_ID_TEMPERATURE:
    case VS_ID_TEMPERATURE_WAKEUP:
    case VS_ID_AMBIENT_TEMPERATURE:
    case VS_ID_AMBIENT_TEMPERATURE_WAKEUP:
        if (cursor->remaining < _fifoSizes[BHY_DATA_TYPE_SCALAR_S16])
        {
            return BHY_OUT_OF_RANGE;
        }

        (*fifo_data_type) = BHY_DATA_TYPE_SCALAR_S16;
        fifo_data_output->data_scalar_s16.sensor_id = *p;
        fifo_data_output->data_scalar_s16.data = read_i16_le(&p[1]);
        break;

    case VS_ID_BAROMETER:
    case VS_ID_BAROMETER_WAKEUP:
        if (cursor->remaining < _fifoSizes[BHY_DATA_TYPE_SCALAR_U24])
        {
            return BHY_OUT_OF_RANGE;
        }

        (*fifo_data_type) = BHY_DATA_TYPE_SCALAR_U24;
        fifo_data_output->data_scalar_u24.sensor_id = *p;
        fifo_data_output->data_scalar_u24.data = read_u24_le(&p[1]);
        break;

    case VS_ID_SIGNIFICANT_MOTION:
    case VS_ID_SIGNIFICANT_MOTION_WAKEUP:
    case VS_ID_STEP_DETECTOR:
    case VS_ID_STEP_DETECTOR_WAKEUP:
    case VS_ID_TILT_DETECTOR:
    case VS_ID_TILT_DETECTOR_WAKEUP:
    case VS_ID_WAKE_GESTURE:
        // gu8WakeFlag = 1;
        break;
    case VS_ID_WAKE_GESTURE_WAKEUP:
        // quaternion->gu8WakeFlag = 1;
        // write_buffer.gu8WakeFlag = 1;
        // gu8WakeFlag = 1;
        break;
    case VS_ID_GLANCE_GESTURE:
    case VS_ID_GLANCE_GESTURE_WAKEUP:
    case VS_ID_PICKUP_GESTURE:
    case VS_ID_PICKUP_GESTURE_WAKEUP:
        (*fifo_data_type) = BHY_DATA_TYPE_SENSOR_EVENT;
        fifo_data_output->data_sensor_event.sensor_id = *p;
        break;

    case VS_ID_UNCALIBRATED_MAGNETOMETER:
    case VS_ID_UNCALIBRATED_MAGNETOMETER_WAKEUP:
    case VS_ID_UNCALIBRATED_GYROSCOPE:
    case VS_ID_UNCALIBRATED_GYROSCOPE_WAKEUP:
        if (cursor->remaining < _fifoSizes[BHY_DATA_TYPE_UNCALIB_VECTOR])
        {
            return BHY_OUT_OF_RANGE;
        }

        (*fifo_data_type) = BHY_DATA_TYPE_UNCALIB_VECTOR;
        fifo_data_output->data_uncalib_vector.sensor_id = *p;
        fifo_data_output->data_uncalib_vector.x = read_i16_le(&p[1]);
        fifo_data_output->data_uncalib_vector.y = read_i16_le(&p[3]);
        fifo_data_output->data_uncalib_vector.z = read_i16_le(&p[5]);
        fifo_data_output->data_uncalib_vector.x_bias = read_i16_le(&p[7]);
        fifo_data_output->data_uncalib_vector.y_bias = read_i16_le(&p[9]);
        fifo_data_output->data_uncalib_vector.z_bias = read_i16_le(&p[11]);
        fifo_data_output->data_uncalib_vector.status = p[13];
        break;

    case VS_ID_META_EVENT:
    case VS_ID_META_EVENT_WAKEUP:
        if (cursor->remaining < _fifoSizes[BHY_DATA_TYPE_META_EVENT])
        {
            return BHY_OUT_OF_RANGE;
        }

        (*fifo_data_type) = BHY_DATA_TYPE_META_EVENT;
        fifo_data_output->data_meta_event.meta_event_id = *p;
        fifo_data_output->data_meta_event.event_number = (bhy_meta_event_type_t)p[1];
        fifo_data_output->data_meta_event.sensor_type = p[2];
        fifo_data_output->data_meta_event.event_specific = p[3];
        break;
    case VS_ID_DEBUG:
        if (cursor->remaining < _fifoSizes[BHY_DATA_TYPE_DEBUG])
        {
            return BHY_OUT_OF_RANGE;
        }

        (*fifo_data_type) = BHY_DATA_TYPE_DEBUG;
        fifo_data_output->data_debug.sensor_id = *p;
        for (i = 0; i < sizeof(fifo_data_output->data_debug.data); i++)
            fifo_data_output->data_debug.data[i] = p[1 + i];
        break;
    case VS_ID_BSX_C:
    case VS_ID_BSX_B:
    case VS_ID_BSX_A:
        if (cursor->remaining < _fifoSizes[BHY_DATA_TYPE_BSX])
        {
            return BHY_OUT_OF_RANGE;
        }

        (*fifo_data_type) = BHY_DATA_TYPE_BSX;
        fifo_data_output->data_bsx.sensor_id = *p;
        fifo_data_output->data_bsx.x = read_u32_le(&p[1]);
        fifo_data_output->data_bsx.y = read_u32_le(&p[5]);
        fifo_data_output->data_bsx.z = read_u32_le(&p[9]);
        fifo_data_output->data_bsx.timestamp = read_u32_le(&p[13]);
        break;

    case VS_ID_CUS1:
    case VS_ID_CUS2:
    case VS_ID_CUS3:
    case VS_ID_CUS4:
    case VS_ID_CUS5:
        (*fifo_data_type) = BHY_DATA_TYPE_CUS1 + *p - VS_ID_CUS1;

        if (cursor->remaining < _fifoSizes[*fifo_data_type])
        {
            return BHY_OUT_OF_RANGE;
        }

        fifo_data_output->data_pdr.sensor_id = *p;

        for (i = 0; i < _fifoSizes[*fifo_data_type] - 1; i++)
            fifo_data_output->data_custom.data[i] = p[i];
        break;

    case VS_ID_CUS1_WAKEUP:
    case VS_ID_CUS2_WAKEUP:
    case VS_ID_CUS3_WAKEUP:
    case VS_ID_CUS4_WAKEUP:
    case VS_ID_CUS5_WAKEUP:
        (*fifo_data_type) = BHY_DATA_TYPE_CUS1 + *p - VS_ID_CUS1_WAKEUP;

        if (cursor->remaining < _fifoSizes[*fifo_data_type])
        {
            return BHY_OUT_OF_RANGE;
        }

        fifo_data_output->data_pdr.sensor_id = *p;

        for (i = 0; i < _fifoSizes[*fifo_data_type] - 1; i++)
            fifo_data_output->data_custom.data[i] = p[i];
        break;

        /* the VS sensor ID is unknown. Either the sync has been lost or the */
        /* ram patch implements a new sensor ID that this driver doesn't yet */
        /* support                               */
    default:
        return BHY_OUT_OF_RANGE;
    }

    cursor->ptr += _fifoSizes[*fifo_data_type];
    cursor->remaining -= _fifoSizes[*fifo_data_type];

    return BHY_SUCCESS;
}

/*
 *---------------------------------------------------------------------------------------------
 * Function:      IMU_set_wakeup_sensor_configuration
 * Description:   This function is used to set the Wakeup configuration of the
 sensorx
 *
 * Parameters:     [in1] = mxc_i2c_regs_t* i2c_inst 			(i2C
 instance)
 * 				  [in2] = sensor_configuration
 (Sensor Configuration structure)
 * 				  [in3] = uint8_t v_parameter_request_u8
 (parameters to write) [in4] = *sensor_callback                  (Callback
 function)
 *
 * return          IMU Transaction status
 * ---------------------------------------------------------------------------------------------
 */

int8_t IMU_set_wakeup_sensor_configuration(mxc_i2c_regs_t *i2c_inst,
                                           struct sensor_configuration_wakeup_t sensor_configuration,
                                           uint8_t v_parameter_request_u8)
{
    /* variable used for return the status of communication result*/
    int8_t com_rslt = BHY_COMM_RES;
    /* write sample rate*/
    write_buffer.write_parameter_byte1 = (uint8_t)(sensor_configuration.wakeup_sample_rate & BHY_MASK_LSB_DATA);
    write_buffer.write_parameter_byte2 =
        (uint8_t)((sensor_configuration.wakeup_sample_rate & BHY_MASK_MSB_DATA) >> BHY_SHIFT_BIT_POSITION_BY_08_BITS);
    /* write maximum report latency*/
    write_buffer.write_parameter_byte3 = (uint8_t)(sensor_configuration.wakeup_max_report_latency & BHY_MASK_LSB_DATA);
    write_buffer.write_parameter_byte4 =
        (uint8_t)((sensor_configuration.wakeup_max_report_latency & BHY_MASK_MSB_DATA) >>
                  BHY_SHIFT_BIT_POSITION_BY_08_BITS);
    /* write change sensitivity*/
    write_buffer.write_parameter_byte5 = (uint8_t)(sensor_configuration.wakeup_change_sensitivity & BHY_MASK_LSB_DATA);
    write_buffer.write_parameter_byte6 =
        (uint8_t)((sensor_configuration.wakeup_change_sensitivity & BHY_MASK_MSB_DATA) >>
                  BHY_SHIFT_BIT_POSITION_BY_08_BITS);
    /* write dynamic range*/
    write_buffer.write_parameter_byte7 = (uint8_t)(sensor_configuration.wakeup_dynamic_range & BHY_MASK_LSB_DATA);
    write_buffer.write_parameter_byte8 =
        (uint8_t)((sensor_configuration.wakeup_dynamic_range & BHY_MASK_MSB_DATA) >> BHY_SHIFT_BIT_POSITION_BY_08_BITS);
    /* load the parameter of wakeup sensor configuration*/
    com_rslt = IMU_write_parameter_bytes(i2c_inst, BHY_PAGE_3, v_parameter_request_u8);

    return com_rslt;
}

/*
 *---------------------------------------------------------------------------------------------
 * Function:      IMU_set_non_wakeup_sensor_configuration
 * Description:   This function is used to set the Non Wakeup configuration of
 the sensor
 *
 * Parameters:     [in1] = mxc_i2c_regs_t* i2c_inst 			(i2C
 instance)
 * 				  [in2] = sensor_configuration
 (Sensor Configuration structure)
 * 				  [in3] = uint8_t v_parameter_request_u8
 (parameters to write) [in4] = *sensor_callback                  (Callback
 function)
 *
 * return          IMU Transaction status
 * ---------------------------------------------------------------------------------------------
 */
int8_t IMU_set_non_wakeup_sensor_configuration(mxc_i2c_regs_t *i2c_inst,
                                               struct sensor_configuration_non_wakeup_t sensor_configuration,
                                               uint8_t v_parameter_request_u8)
{
    /* variable used for return the status of communication result*/
    int8_t com_rslt = BHY_COMM_RES;
    /* write sample rate*/
    write_buffer.write_parameter_byte1 = (uint8_t)(sensor_configuration.non_wakeup_sample_rate & BHY_MASK_LSB_DATA);
    write_buffer.write_parameter_byte2 = (uint8_t)((sensor_configuration.non_wakeup_sample_rate & BHY_MASK_MSB_DATA) >>
                                                   BHY_SHIFT_BIT_POSITION_BY_08_BITS);
    /* write maximum report latency*/
    write_buffer.write_parameter_byte3 =
        (uint8_t)(sensor_configuration.non_wakeup_max_report_latency & BHY_MASK_LSB_DATA);
    write_buffer.write_parameter_byte4 =
        (uint8_t)((sensor_configuration.non_wakeup_max_report_latency & BHY_MASK_MSB_DATA) >>
                  BHY_SHIFT_BIT_POSITION_BY_08_BITS);
    /* write sensitivity*/
    write_buffer.write_parameter_byte5 =
        (uint8_t)(sensor_configuration.non_wakeup_change_sensitivity & BHY_MASK_LSB_DATA);
    write_buffer.write_parameter_byte6 =
        (uint8_t)((sensor_configuration.non_wakeup_change_sensitivity & BHY_MASK_MSB_DATA) >>
                  BHY_SHIFT_BIT_POSITION_BY_08_BITS);
    /* write dynamic range*/
    write_buffer.write_parameter_byte7 = (uint8_t)(sensor_configuration.non_wakeup_dynamic_range & BHY_MASK_LSB_DATA);
    write_buffer.write_parameter_byte8 =
        (uint8_t)((sensor_configuration.non_wakeup_dynamic_range & BHY_MASK_MSB_DATA) >>
                  BHY_SHIFT_BIT_POSITION_BY_08_BITS);
    /* load the parameter of non wakeup sensor configuration*/
    com_rslt = IMU_write_parameter_bytes(i2c_inst, BHY_PAGE_3, v_parameter_request_u8);

    return com_rslt;
}

/*
 *---------------------------------------------------------------------------------------------
 * Function:      IMU_write_parameter_bytes
 * Description:   This function is used to write the parameter bytes to IMU
 * sensor
 *
 * Parameters:     [in1] = mxc_i2c_regs_t* i2c_inst 			(i2C
 * instance) [in2] = uint8_t v_page_select_u8			(page address)
 * 				  [in3] =  uint8_t v_parameter_request_u8
 *	(parameters to write)
 *
 * return          IMU Transaction status
 * ---------------------------------------------------------------------------------------------
 */
int8_t IMU_write_parameter_bytes(mxc_i2c_regs_t *i2c_inst, uint8_t v_page_select_u8, uint8_t v_parameter_request_u8)
{
    /* variable used for return the status of communication result*/
    int8_t com_rslt = BHY_COMM_RES;
    uint8_t v_parameter_ack_u8 = BHY_INIT_VALUE;
    uint8_t v_parameter_ack_check_u8 = BHY_INIT_VALUE;
    uint8_t v_write_parameter_byte_u8[BHY_WRITE_BUFFER_SIZE];
    uint8_t init_array_data = BHY_INIT_VALUE;

    for (; init_array_data < BHY_WRITE_BUFFER_SIZE; init_array_data++)
    {
        v_write_parameter_byte_u8[init_array_data] = BHY_INIT_VALUE;
    }
    v_write_parameter_byte_u8[BHY_WRITE_BUFFER_1_REG] = write_buffer.write_parameter_byte1;
    v_write_parameter_byte_u8[BHY_WRITE_BUFFER_2_REG] = write_buffer.write_parameter_byte2;
    v_write_parameter_byte_u8[BHY_WRITE_BUFFER_3_REG] = write_buffer.write_parameter_byte3;
    v_write_parameter_byte_u8[BHY_WRITE_BUFFER_4_REG] = write_buffer.write_parameter_byte4;
    v_write_parameter_byte_u8[BHY_WRITE_BUFFER_5_REG] = write_buffer.write_parameter_byte5;
    v_write_parameter_byte_u8[BHY_WRITE_BUFFER_6_REG] = write_buffer.write_parameter_byte6;
    v_write_parameter_byte_u8[BHY_WRITE_BUFFER_7_REG] = write_buffer.write_parameter_byte7;
    v_write_parameter_byte_u8[BHY_WRITE_BUFFER_8_REG] = write_buffer.write_parameter_byte8;
    /* write values to the load address*/
    com_rslt = IMU_Write_Register(i2c_inst, BHY_I2C_REG_PARAMETER_WRITE_BUFFER_ZERO,
                                  &v_write_parameter_byte_u8[BHY_WRITE_BUFFER_1_REG], BHY_WRITE_BUFFER_SIZE);
    if (com_rslt != BHY_SUCCESS)
        return com_rslt;
    /* select the page*/
    com_rslt = IMU_set_parameter_page_select(i2c_inst, v_page_select_u8);
    if (com_rslt != BHY_SUCCESS)
        return com_rslt;
    /* select the parameter*/
    com_rslt = IMU_Set_parameter_request(i2c_inst, v_parameter_request_u8);
    if (com_rslt != BHY_SUCCESS)
        return com_rslt;
    for (v_parameter_ack_check_u8 = BHY_INIT_VALUE; v_parameter_ack_check_u8 < BHY_PARAMETER_ACK_LENGTH;
         v_parameter_ack_check_u8++)
    {
        /* read the acknowledgement */
        com_rslt = IMU_get_parameter_acknowledge(i2c_inst, &v_parameter_ack_u8);
        if (com_rslt != BHY_SUCCESS)
            return com_rslt;
        if (v_parameter_ack_u8 == v_parameter_request_u8)
        {
            com_rslt += BHY_SUCCESS;
            break;
        }
        else if (v_parameter_ack_u8 == BHY_PARAMETER_ACK_CHECK)
        {
            com_rslt += BHY_ERROR;
        }
    }

    /* end the parameter transfer (datasheet Config Parameter I/O procedure) */
    com_rslt += IMU_Set_parameter_request(i2c_inst, 0);

    return com_rslt;
}

/*
 *---------------------------------------------------------------------------------------------
 * Function:      IMU_set_parameter_page_select
 * Description:   This function is used to set the parameters page select
 *
 * Parameters:     [in1] = mxc_i2c_regs_t* i2c_inst 			(i2C
 * instance) [in2] = uint8_t v_page_select_u8			(page address)
 *
 * return          IMU Transaction status
 * ---------------------------------------------------------------------------------------------
 */
int8_t IMU_set_parameter_page_select(mxc_i2c_regs_t *i2c_inst, uint8_t v_page_select_u8)
{
    /* variable used for return the status of communication result*/
    int8_t com_rslt = BHY_COMM_RES;
    uint8_t v_data_u8 = BHY_INIT_VALUE;

    if (BHY_NULL_PTR == p_bhy)
    {
        com_rslt = BHY_NULL;
    }
    else
    {
        v_data_u8 = v_page_select_u8;
        /* read the parameter page information*/
        com_rslt = p_bhy->BHY_BUS_WRITE_FUNC(i2c_inst, p_bhy->device_addr, BHY_I2C_REG_PARAMETER_PAGE_SELECT__REG,
                                             &v_data_u8, BHY_GEN_READ_WRITE_LENGTH);
    }
    return com_rslt;
}

/*
 *---------------------------------------------------------------------------------------------
 * Function:      IMU_Set_parameter_request
 * Description:   This function is used to set the parameters request
 *
 * Parameters:     [in1] = mxc_i2c_regs_t* i2c_inst 			(i2C
 * instance) [in2] = uint8_t v_parameter_request_u8	(parameters)
 *
 * return          IMU Transaction status
 * ---------------------------------------------------------------------------------------------
 */
int8_t IMU_Set_parameter_request(mxc_i2c_regs_t *i2c_inst, int8_t v_parameter_request_u8)
{
    /* variable used for return the status of communication result*/
    int8_t com_rslt = BHY_COMM_RES;
    uint8_t v_data_u8 = BHY_INIT_VALUE;
    v_data_u8 = v_parameter_request_u8;

    if (BHY_NULL_PTR == p_bhy)
    {
        com_rslt = BHY_NULL;
    }
    else
    {
        /* write load parameter request*/
        v_data_u8 = v_parameter_request_u8;
        com_rslt = p_bhy->BHY_BUS_WRITE_FUNC(i2c_inst, p_bhy->device_addr, BHY_I2C_REG_LOAD_PARAMETER_REQUEST__REG,
                                             &v_data_u8, BHY_GEN_READ_WRITE_LENGTH);
    }
    return com_rslt;
}

/*
 *---------------------------------------------------------------------------------------------
 * Function:      IMU_get_parameter_acknowledge
 * Description:   This function is used to read the parameter acknowdlegement
 *
 * Parameters:     [in1] = mxc_i2c_regs_t* i2c_inst 			    (i2C
 * instance) [in2] = uint8_t *v_parameter_acknowledge_u8	(aknowledgement)
 *
 * return          IMU Transaction status
 * ---------------------------------------------------------------------------------------------
 */
int8_t IMU_get_parameter_acknowledge(mxc_i2c_regs_t *i2c_inst, uint8_t *v_parameter_acknowledge_u8)
{
    int8_t com_rslt = BHY_COMM_RES;
    uint8_t v_data_u8 = BHY_INIT_VALUE;

    if (BHY_NULL_PTR == p_bhy)
    {
        com_rslt = BHY_NULL;
    }
    else
    {
        /* read the parameter acknowledgement*/
        com_rslt = p_bhy->BHY_BUS_READ_FUNC(i2c_inst, p_bhy->device_addr, BHY_I2C_REG_PARAMETER_ACKNOWLEDGE_ADDR,
                                            &v_data_u8, BHY_GEN_READ_WRITE_LENGTH);
        *v_parameter_acknowledge_u8 = v_data_u8;
    }
    return com_rslt;
}

/*
 *---------------------------------------------------------------------------------------------
 * Function:      IMU_read_bytes_remaining
 * Description:   This function is used to read remaining bytes
 *
 * Parameters:     [in1] = mxc_i2c_regs_t* i2c_inst 			    (i2C
 * instance) [in2] = uint8_t *v_bytes_remaining_u16	    (Bytes remaining)
 *
 * return          IMU Transaction status
 * ---------------------------------------------------------------------------------------------
 */
int8_t IMU_read_bytes_remaining(mxc_i2c_regs_t *i2c_inst, uint16_t *v_bytes_remaining_u16)
{
    int8_t com_rslt = BHY_COMM_RES;

    /* Array contains the bytes reaming of FIFO lSB and MSB data
        v_data_u8[LSB_ZERO] - LSB
        v_data_u8[MSB_ONE] - MSB*/
    uint8_t v_data_u8[BHY_BYTES_REMAINING_SIZE] = {BHY_INIT_VALUE, BHY_INIT_VALUE};

    if (BHY_NULL_PTR == p_bhy)
    {
        com_rslt = BHY_NULL;
    }
    else
    {
        /* read bytes remaining data */
        com_rslt = p_bhy->BHY_BUS_READ_FUNC(i2c_inst, p_bhy->device_addr, BHY_I2C_REG_BYTES_REMAINING_LSB_ADDR,
                                            v_data_u8, BHY_BYTES_REMAINING_LENGTH);

        /* get the bytes remaining data*/
        *v_bytes_remaining_u16 = (uint16_t)((v_data_u8[BHY_BYTES_REMAINING_MSB] << BHY_SHIFT_BIT_POSITION_BY_08_BITS) |
                                       (v_data_u8[BHY_BYTES_REMAINING_LSB]));
    }
    return com_rslt;
}

/*
 *---------------------------------------------------------------------------------------------
 * Function:      IMU_write_reg
 * Description:   This function is used to read remaining bytes
 *
 * Parameters:     [in1] = mxc_i2c_regs_t* i2c_inst 			(i2C
 * instance) [in2] = uint8_t v_addr_u8 	(Address) [in3] = uint8_t *v_data_u8
 *			(Data) [in4] = uint16_t v_len_u16
 *	(Length)
 *
 * return          IMU Transaction status
 * ---------------------------------------------------------------------------------------------
 */
int8_t IMU_write_reg(mxc_i2c_regs_t *i2c_inst, uint8_t v_addr_u8, uint8_t *v_data_u8, uint16_t v_len_u16)
{
    /* variable used for return the status of communication result*/
    int8_t com_rslt = BHY_COMM_RES;
    if (BHY_NULL_PTR == p_bhy)
    {
        com_rslt = BHY_NULL;
    }
    else
    {
        com_rslt = p_bhy->BHY_BUS_WRITE_FUNC(i2c_inst, p_bhy->device_addr, v_addr_u8, v_data_u8, v_len_u16);
    }
    return com_rslt;
}

/*
 *---------------------------------------------------------------------------------------------
 * Function:      IMU_write_parameter_page
 * Description:   This function is used to write the parameters to the memory
 * page address
 *
 * Parameters:     [in1] = mxc_i2c_regs_t* i2c_inst 			(i2C
 * instance) [in2] = uint8_t v_addr_u8 	(Address) [in3] = uint8_t *v_data_u8
 *			(Data) [in4] = uint16_t v_len_u16
 *	(Length)
 *
 * return          IMU Transaction status
 * ---------------------------------------------------------------------------------------------
 */
int8_t IMU_write_parameter_page(mxc_i2c_regs_t *i2c_inst, uint8_t page, uint8_t parameter, uint8_t *data,
                                uint8_t length)
{
    /* variable used for return the status of communication result */
    int8_t com_rslt = BHY_COMM_RES;

    uint8_t v_parameter_ack = BHY_INIT_VALUE;
    uint8_t v_parameter_ack_check = BHY_INIT_VALUE;

    /* write values to the load address*/
    if (length > MAX_WRITE_BYTES)
    {
        length = MAX_WRITE_BYTES;
    }
    else if (length == 0)
    {
        return BHY_SUCCESS;
    }
    com_rslt = IMU_write_reg(i2c_inst, BHY_I2C_REG_PARAMETER_WRITE_BUFFER_ZERO, data, length);

    /* select the page*/
    if (page > MAX_PAGE_NUM)
    {
        page = MAX_PAGE_NUM;
    }
    else if (page == 0)
    {
        return BHY_SUCCESS;
    }

    page = ((length & 0x07) << 4) | page;
    com_rslt += IMU_write_reg(i2c_inst, BHY_I2C_REG_PARAMETER_PAGE_SELECT__REG, &page, 1);

    /* select the parameter*/
    parameter |= 0x80;
    com_rslt += IMU_Set_parameter_request(i2c_inst, parameter);
    for (v_parameter_ack_check = BHY_INIT_VALUE; v_parameter_ack_check < BHY_PARAMETER_ACK_LENGTH;
         v_parameter_ack_check++)
    {
        /* read the acknowledgment*/
        com_rslt += IMU_get_parameter_acknowledge(i2c_inst, &v_parameter_ack);
        if (v_parameter_ack == parameter)
        {
            com_rslt += BHY_SUCCESS;
            break;
        }
        else if (v_parameter_ack == BHY_PARAMETER_ACK_CHECK)
        {
            com_rslt += BHY_ERROR;
        }
    }

    /* end the parameter transfer (datasheet Config Parameter I/O procedure) */
    com_rslt += IMU_Set_parameter_request(i2c_inst, 0);

    return com_rslt;
}

/*
 *---------------------------------------------------------------------------------------------
 * Function:      IMU_read_parameter_page
 * Description:   This function is used to read the parameters from memory page
 * address
 *
 * Parameters:     [in1] = mxc_i2c_regs_t* i2c_inst 			(i2C
 * instance) [in2] = uint8_t page  (Page Address) [in3] = uint8_t parameter
 *			(parameter) [in4] = uint8_t *data
 *		(data) [in5] = uint8_t length
 *	(length)
 *
 * return          IMU Transaction status
 * ---------------------------------------------------------------------------------------------
 */
int8_t IMU_read_parameter_page(mxc_i2c_regs_t *i2c_inst, uint8_t page, uint8_t parameter, uint8_t *data, uint8_t length)
{
    /* variable used for return the status of communication result*/
    int8_t com_rslt = BHY_COMM_RES;

    uint8_t v_parameter_ack = BHY_INIT_VALUE;
    uint8_t v_parameter_ack_check = BHY_INIT_VALUE;

    if (length > 16)
    {
        length = 16;
    }
    else if (length == 0)
    {
        return BHY_SUCCESS;
    }
    /* select the page*/
    if (page > 15)
    {
        page = 15;
    }
    else if (page == 0)
    {
        return BHY_SUCCESS;
    }

    page = ((length & 0x07) << 4) | page;
    com_rslt = IMU_write_reg(i2c_inst, BHY_I2C_REG_PARAMETER_PAGE_SELECT__REG, &page, 1);

    /* select the parameter*/
    parameter &= 0x7F;
    com_rslt += IMU_Set_parameter_request(i2c_inst, parameter);
    for (v_parameter_ack_check = BHY_INIT_VALUE; v_parameter_ack_check < BHY_PARAMETER_ACK_LENGTH;
         v_parameter_ack_check++)
    {
        /* read the acknowledgment*/
        com_rslt += IMU_get_parameter_acknowledge(i2c_inst, &v_parameter_ack);
        if (v_parameter_ack == parameter)
        {
            com_rslt += BHY_SUCCESS;
            break;
        }
        else if (v_parameter_ack == BHY_PARAMETER_ACK_CHECK)
        {
            com_rslt += BHY_ERROR;
        }
    }
    /* read values to the load address*/
    com_rslt += IMU_read_reg(i2c_inst, BHY_I2C_REG_PARAMETER_READ_BUFFER_ZERO, data, length);

    /* end the parameter transfer (datasheet Config Parameter I/O procedure) */
    com_rslt += IMU_set_parameter_page_select(i2c_inst, 0);

    return com_rslt;
}

/*
 *---------------------------------------------------------------------------------------------
 * Function:      IMU_read_fifo
 * Description:   This function is used to read the fifo
 *
 * Parameters:     [in1] = mxc_i2c_regs_t* i2c_inst 			(i2C
 * instance) [in2] = uint8_t *buffer   				(Buffer to
 * store) [in3] = uint16_t buffer_size   			(Buffer Size)
 * 				  [in4] = uint16_t *bytes_read
 *	(Bytes to read) [in5] = uint16_t *bytes_left  			(Bytes
 * left in fifo)
 *
 * return          IMU Transaction status
 * ---------------------------------------------------------------------------------------------
 */
int8_t IMU_read_fifo(mxc_i2c_regs_t *i2c_inst, uint8_t *buffer, uint16_t size)
{
    return IMU_read_reg(i2c_inst, 0x00, buffer, size);
}

/*
 *---------------------------------------------------------------------------------------------
 * Function:      IMU_abort_fifo_transfer
 * Description:   Sets then clears the Abort Transfer bit (Host Interface
 *                Control register, 0x55). Makes the sensor discard whatever
 *                FIFO backlog it already announced via Bytes Remaining and
 *                reset it to 0, so the host can recover instead of being
 *                stuck re-announcing the same backlog forever.
 *
 * Parameters:     [in1] = mxc_i2c_regs_t* i2c_inst 			(i2C
 * instance)
 *
 * return          IMU Transaction status
 * ---------------------------------------------------------------------------------------------
 */
int8_t IMU_abort_fifo_transfer(mxc_i2c_regs_t *i2c_inst)
{
    int8_t com_rslt = BHY_COMM_RES;
    uint8_t v_data_u8 = BHY_INIT_VALUE;

    if (BHY_NULL_PTR == p_bhy)
    {
        return BHY_NULL;
    }

    com_rslt = p_bhy->BHY_BUS_READ_FUNC(i2c_inst, p_bhy->device_addr,
                                        BHY_I2C_REG_HOST_INTERFACE_CONTROL_ABORT_TRANSFER__REG, &v_data_u8,
                                        BHY_GEN_READ_WRITE_LENGTH);
    if (BHY_SUCCESS != com_rslt)
    {
        return com_rslt;
    }

    v_data_u8 = BHY_SET_BITSLICE(v_data_u8, BHY_I2C_REG_HOST_INTERFACE_CONTROL_ABORT_TRANSFER, 1);
    com_rslt = p_bhy->BHY_BUS_WRITE_FUNC(i2c_inst, p_bhy->device_addr,
                                         BHY_I2C_REG_HOST_INTERFACE_CONTROL_ABORT_TRANSFER__REG, &v_data_u8,
                                         BHY_GEN_READ_WRITE_LENGTH);

    /* must not clear Abort Transfer immediately after setting it (datasheet
     * 10.12); the read/write pair above already spends that time on the bus */
    v_data_u8 = BHY_SET_BITSLICE(v_data_u8, BHY_I2C_REG_HOST_INTERFACE_CONTROL_ABORT_TRANSFER, 0);
    com_rslt += p_bhy->BHY_BUS_WRITE_FUNC(i2c_inst, p_bhy->device_addr,
                                          BHY_I2C_REG_HOST_INTERFACE_CONTROL_ABORT_TRANSFER__REG, &v_data_u8,
                                          BHY_GEN_READ_WRITE_LENGTH);

    return com_rslt;
}

/*
 *---------------------------------------------------------------------------------------------
 * Function:      sensor_i2c_write
 * Description:   This function is an I2C Write
 *
 * Parameters:     [in1] = mxc_i2c_regs_t* i2c_inst 			(i2C
 * instance) [in2] = uint8_t addr  (Device Address) [in3] = uint8_t reg
 *				(Register address) [in4] = uint8_t *p_buf
 *				(data to write) [in5] = uint16_t size
 *			(length to write)
 *
 * return          IMU Transaction status
 * ---------------------------------------------------------------------------------------------
 */
int8_t sensor_i2c_write(mxc_i2c_regs_t *i2c_inst, uint8_t addr, uint8_t reg, uint8_t *p_buf, uint16_t size)
{
	int8_t result  = I2C_write(i2c_inst, addr, (uint16_t)reg, p_buf, size);
    MXC_Delay(MXC_DELAY_USEC(BHY_I2C_XFER_DELAY_US));
    if (result != E_SUCCESS)
        return BHY_ERROR;
    else
        return BHY_SUCCESS;
}

/*
 *---------------------------------------------------------------------------------------------
 * Function:      sensor_i2c_read
 * Description:   This function is an I2C read
 *
 * Parameters:     [in1] = mxc_i2c_regs_t* i2c_inst 			(i2C
 * instance) [in2] = uint8_t addr  (Device Address) [in3] = uint8_t reg
 *				(Register address) [in4] = uint8_t *p_buf
 *				(data to read) [in5] = uint16_t size
 *			(length to read)
 *
 * return          IMU Transaction status
 * ---------------------------------------------------------------------------------------------
 */
int8_t sensor_i2c_read(mxc_i2c_regs_t *i2c_inst, uint8_t addr, uint8_t reg, uint8_t *p_buf, uint16_t size)
{
	int8_t result  = I2C_read(i2c_inst, addr, (uint16_t)reg, p_buf, size);
    MXC_Delay(MXC_DELAY_USEC(BHY_I2C_XFER_DELAY_US));
    if (result != E_SUCCESS)
        return BHY_ERROR;
    else
        return BHY_SUCCESS;
}
