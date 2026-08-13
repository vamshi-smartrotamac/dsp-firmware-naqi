/**************************************************************************
 * Copyright (c) 2025 Naqi Logix Inc. All Rights Reserved.
 *
 * File: ComInterface.h
 *
 * This file is part of the proprietary firmware and is protected by
 * applicable copyright laws and international treaties. Unauthorized use,
 * reproduction, or distribution of this file or its contents is strictly
 * prohibited. For permissions, contact info @ naqilogix.com.
 **************************************************************************/

#pragma once

#include "Communication.h"
#include "ExGacquisition.h"
#include "IMUacquisitionInterface.h"

#define PACKET_TYPE_EXG 0xAA
#define PACKET_TYPE_IMU 0xBB

/**
 * @brief Builds EXG packet (ExG + I/Q + LeadOff + Gesture)
 *        from the given EXG sample message and enqueues it for UART
 *        transmission. Owns its own packet sequence counter.
 *
 * @param msg  EXG sample message to packetize
 * @return E_NO_ERROR on success, error code on failure
 */
int ComItf_sendExgData(ExGSamples_t *msg);

/**
 * @brief Builds an IMU packet (quaternions + acc/gyr motion samples + mag)
 *        from the given IMU sample batch and enqueues it for UART
 *        transmission. Owns its own packet sequence counter.
 *
 * @param samples  IMU samples to packetize
 * @return E_NO_ERROR on success, error code on failure
 */
int ComItf_sendImuData(ImuSamples_t *samples);
