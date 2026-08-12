/**************************************************************************
 * Copyright (c) 2025 Naqi Logix Inc. All Rights Reserved.
 *
 * File: ComInterface.c
 *
 * This file is part of the proprietary firmware and is protected by
 * applicable copyright laws and international treaties. Unauthorized use,
 * reproduction, or distribution of this file or its contents is strictly
 * prohibited. For permissions, contact info @ naqilogix.com.
 **************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>

#include "ComInterface.h"
#include "GestureProcessing.h"

/* Global variables ---------------------------------------------------------*/
typedef struct __attribute__((__packed__))
{
    uint8_t  type;
    uint8_t  num;
    int32_t  exg[SAMPLES_PER_MESSAGE];
    uint8_t  lead_off_status;
    int32_t  I_value;
    int32_t  Q_value;
    uint8_t  gesture_out;
    uint32_t checksum;
} PacketExg_t;

typedef struct __attribute__((__packed__))
{
    uint8_t type;
    uint8_t num;
    uint8_t calib_status;
    struct
    {
        float x, y, z, w;
    } quat[IMU_SAMPLE_COUNT];
    struct
    {
        struct
        {
            int16_t x, y, z;
        } acc;
        struct
        {
            int16_t x, y, z;
        } gyr;
    } motion[IMU_SAMPLE_COUNT];
    struct
    {
        int16_t x, y, z;
    } mag;
    uint32_t checksum;
} PacketImu_t;

_Static_assert(sizeof(PacketExg_t) <= COM_TX_PACKET_MAX_SIZE, "PacketExg_t exceeds COM_TX_PACKET_MAX_SIZE");
_Static_assert(sizeof(PacketImu_t) <= COM_TX_PACKET_MAX_SIZE, "PacketImu_t exceeds COM_TX_PACKET_MAX_SIZE");

/* Private function prototypes --------------------------------------------------*/

/*
 *---------------------------------------------------------------------------------------------
 * Function:      compute_checksum
 * Description:   Additive checksum over a byte range (sum of all bytes).
 * Parameters:    [In] const void *data   (start of the range)
 *                [In] size_t len         (number of bytes to sum)
 * return         The checksum
 *  ---------------------------------------------------------------------------------------------
 */
static uint32_t compute_checksum(const void *data, size_t len)
{
    uint32_t       checksum = 0;
    const uint8_t *bytes = data;

    for (size_t i = 0; i < len; i++)
    {
        checksum += bytes[i];
    }

    return checksum;
}

int ComItf_sendExgData(ExGSamples_t *msg)
{
    static uint8_t packet_num = 0;
    ComTxMessage_t txMsg;
    PacketExg_t   *pkt = (PacketExg_t *)txMsg.data;

    memset(pkt, 0, sizeof(PacketExg_t));

    pkt->type = PACKET_TYPE_EXG;
    pkt->num = packet_num++;

    memcpy(pkt->exg, msg->data, sizeof(pkt->exg));

    pkt->lead_off_status = msg->leadOffStatus;
    pkt->I_value = msg->leadOffI;
    pkt->Q_value = msg->leadOffQ;

    pkt->gesture_out = Gproc_getGesture();

    pkt->checksum = compute_checksum(pkt, sizeof(PacketExg_t) - sizeof(pkt->checksum));

    txMsg.len = sizeof(PacketExg_t);

    return Com_AddToQueue(&txMsg);
}

int ComItf_sendImuData(ImuSamples_t *msg)
{
    static uint8_t packet_num = 0;
    ComTxMessage_t txMsg;
    PacketImu_t   *pkt = (PacketImu_t *)txMsg.data;

    memset(pkt, 0, sizeof(PacketImu_t));

    pkt->type = PACKET_TYPE_IMU;
    pkt->num = packet_num++;
    pkt->calib_status = msg->accuracy;

    for (int i = 0; i < IMU_SAMPLE_COUNT; i++)
    {
        pkt->quat[i].x = msg->quat[i].x;
        pkt->quat[i].y = msg->quat[i].y;
        pkt->quat[i].z = msg->quat[i].z;
        pkt->quat[i].w = msg->quat[i].w;

        pkt->motion[i].acc.x = msg->acc[i][0];
        pkt->motion[i].acc.y = msg->acc[i][1];
        pkt->motion[i].acc.z = msg->acc[i][2];

        pkt->motion[i].gyr.x = msg->gyr[i][0];
        pkt->motion[i].gyr.y = msg->gyr[i][1];
        pkt->motion[i].gyr.z = msg->gyr[i][2];
    }

    pkt->mag.x = msg->mag[0];
    pkt->mag.y = msg->mag[1];
    pkt->mag.z = msg->mag[2];

    pkt->checksum = compute_checksum(pkt, sizeof(PacketImu_t) - sizeof(pkt->checksum));

    txMsg.len = sizeof(PacketImu_t);

    return Com_AddToQueue(&txMsg);
}
