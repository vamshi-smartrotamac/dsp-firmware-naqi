/**
 * @file IMUacquisitionInterface.h
 * @author pierre@naqilogix.com
 * @brief Thread-safe interface layer over the BHI160 driver.
 */

#ifndef IMU_ACQUISITION_INTERFACE_H_
#define IMU_ACQUISITION_INTERFACE_H_

#include "i2c.h"
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    float w;
    float x;
    float y;
    float z;
} Quaternion;

#define IMU_SAMPLE_COUNT 2

typedef struct {
    Quaternion quat[IMU_SAMPLE_COUNT];
    int16_t acc[IMU_SAMPLE_COUNT][3];
    int16_t gyr[IMU_SAMPLE_COUNT][3];
    int16_t mag[3];
    uint8_t accuracy;
} ImuSamples_t;

typedef enum {
    IMU_SAMPLE_NONE = 0, /* padding, or other unhandled sensor — ignore */
    IMU_SAMPLE_ACC,
    IMU_SAMPLE_GYR,
    IMU_SAMPLE_MAG,
    IMU_SAMPLE_QUAT,
    IMU_SAMPLE_OVERFLOW,
} ImuSampleType_t;

typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
} ImuVector_t;

typedef struct {
    ImuSampleType_t type;
    union {
        ImuVector_t vector;
        struct {
            Quaternion orientation;
            uint8_t accuracy;
        } quaternion;
    };
} ImuSampleEvent_t;

/**
 * @brief Initialize the BHI160: upload firmware, configure sensors, register callbacks.
 * @return 0 on success, negative on error.
 */
int8_t ImuItf_deviceInit(mxc_i2c_regs_t *i2c_inst);

/**
 * @brief Fetch the next decoded sample from the BHI160, draining fresh bytes
 *        from its FIFO over I2C when the internal backlog is empty.
 * @param out Filled with the next sample's type and payload.
 * @return true if @p out holds a new sample, false once the FIFO backlog is
 *         drained (call again on the next drain cycle).
 */
bool ImuItf_readNextSample(ImuSampleEvent_t *out);

/**
 * @brief Callback invoked (from ISR context) on each BHI160 data-ready
 *        interrupt. Owned by the caller — this layer has no FreeRTOS
 *        dependency, so any task-notification logic belongs in the callback.
 */
typedef void (*ImuInterruptCallback_t)(void);

/**
 * @brief Configure the BHI160 interrupt pin as a level-triggered GPIO
 *        interrupt and register @p callback to run on each data-ready event.
 *        The interrupt is masked on entry and must be re-armed by the caller
 *        (done internally by ImuItf_readNextSample() once the FIFO is drained)
 *        — the BHI160 holds the line asserted as long as data is pending, so
 *        an edge-triggered or unmasked config would storm the CPU.
 * @param callback Called from ISR context whenever the sensor signals new
 *                 FIFO data.
 */
void ImuItf_enableDataReadyInterrupt(ImuInterruptCallback_t callback);

#endif /* IMU_ACQUISITION_INTERFACE_H_ */
