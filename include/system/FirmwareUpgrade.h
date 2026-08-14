/**************************************************************************
 * Copyright (c) 2025 Naqi Logix Inc. All Rights Reserved.
 *
 * File: FirmwareUpgrade.h
 *
 * This file is part of the proprietary firmware and is protected by
 * applicable copyright laws and international treaties. Unauthorized use,
 * reproduction, or distribution of this file or its contents is strictly
 * prohibited. For permissions, contact info @ naqilogix.com.
 **************************************************************************/

#ifndef INC_SYSTEM_FIRMWARE_UPGRADE_H
#define INC_SYSTEM_FIRMWARE_UPGRADE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Checks whether the received bytes match the firmware-upgrade trigger
 *        command. Cheap and non-blocking — safe to call from ISR context.
 *
 * @param buf Received bytes
 * @param len Number of bytes available in buf
 * @return true if the upgrade command was recognized
 */
bool Dfu_isRequested(const uint8_t *buf, size_t len);

/**
 * @brief Erases the upgrade-flag flash page and resets into the bootloader.
 *        Blocks for the duration of the flash erase and never returns —
 *        must be called from task context, never from an ISR.
 */
void Dfu_trigger(void);

#endif /* INC_SYSTEM_FIRMWARE_UPGRADE_H */
