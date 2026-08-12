/**
 * @file IMUacquisition.h
 * @author pierre@wisear.io
 * @brief
 * @version 0.1
 * @date 2026-02-05
 *
 * @copyright Copyright (c) 2026
 *
 */
#ifndef IMUACQUISITION_H
#define IMUACQUISITION_H

#include <stdint.h>
#include "IMUacquisitionInterface.h"

#define ACQUISITION_TASK_NAME_IMU "ImuAcquisition"

void Imu_acquireDataTask(void *pvParameters);

int8_t Imu_initialization(mxc_i2c_regs_t *i2c_inst);

#endif /* IMUACQUISITION_H */
