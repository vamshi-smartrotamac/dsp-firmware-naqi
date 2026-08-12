/**
 * @file IMUacquisitionInterface.c
 * @author pierre@naqilogix.com
 */

#include "IMUacquisitionInterface.h"
#include "bhi160.h"
#include "i2c_driver.h"
#include "board_init.h"
#include "logger.h"
#include "mxc_delay.h"
#include "gpio.h"
#include "nvic_table.h"
#include <math.h>
#include <string.h>

/* ── compile-time config ──────────────────────────────────────────────────── */
#define SAMPLE_RATE 				50                  /* Hz — acc / gyr / quat */
#define MAG_SAMPLE_RATE 			25                  /* Hz — half the acc/gyr/quat rate */
/* Non-zero latency batches samples instead of interrupting on every one */
#define MAG_REPORT_LATENCY_MS 		(1000 * IMU_SAMPLE_COUNT / SAMPLE_RATE)

/* bounds consecutive FIFO-overflow retries so a stuck sensor can't busy-spin this task */
#define IMU_FIFO_OVERFLOW_RETRY_LIMIT 3

/* ── quaternion fixed-point scaling ──────────────────────────────────────── */
/* BHI160 outputs quaternion components as Q14 fixed-point integers:
 * the range [-16384, 16384] maps to the normalised range [-1.0, 1.0]. */
#define BHI160_QUAT_SCALE 			16384.0f     /* 2^14 — Q14 fixed-point divisor  */
#define BHI160_ACCURACY_SCALE 		16384.0f /* same encoding for estimated_accuracy */

/* ── axis remapping matrices ──────────────────────────────────────────────── */
static int8_t imu_mapping[3 * 3] = {0, 0, -1, 1, 0, 0, 0, -1, 0};
static int8_t mag_mapping[3 * 3] = {0, 0, 1, 1, 0, 0, 0, 1, 0};
extern const unsigned char bhy1_firmware[];

/* ── quaternion helpers ───────────────────────────────────────────────────── */

/**
 * @brief Multiplies two quaternions a and b using the Hamilton product formula.
 *       The resulting quaternion represents the combined rotation of a followed by b.
 *       This function is used to apply the mount-correction quaternion to the raw quaternion from the sensor.
 * @param a The first quaternion operand, representing the initial rotation.
 *        b The second quaternion operand, representing the rotation to be applied after a.
 * @return The product of the two quaternions, representing the combined rotation. The components are calculated using the standard quaternion multiplication formula
 */
static Quaternion quaternion_multiply(Quaternion a, Quaternion b)
{
    return (Quaternion){
        .w = a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z,
        .x = a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
        .y = a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,
        .z = a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w,
    };
}

/* Correction quaternion applied at init time (identity = no correction). */
static Quaternion s_q_correction = {1.0, 0.0, 0.0, 0.0};

static Quaternion apply_correction(Quaternion q)
{
    return quaternion_multiply(q, s_q_correction);
}

/* ── I2C instance, memorised at ImuItf_deviceInit() for later FIFO reads ───────── */
static mxc_i2c_regs_t *imuI2cInst = NULL;

/* ── IMU interrupt callback ──────────────────────────────────────────────── */
static ImuInterruptCallback_t dataReadyCallback = NULL;

/* ── FIFO entry → sample dispatch ────────────────────────────────────────────── */

/**
 * @brief   Converts a raw vector FIFO sample (accelerometer, gyroscope, or
 *          magnetometer) into a public ImuSampleEvent_t.
 * @param   sample Raw vector sample from the FIFO (x, y, z, sensor ID).
 * @param   out    Filled with the sample's type and vector payload.
 * @return  true if the sensor ID was recognised and @p out was filled,
 *          false otherwise (caller should keep draining the FIFO).
 */
static bool handle_vector(bhy_data_generic_t *sample, ImuSampleEvent_t *out)
{
    ImuVector_t vector = {
        .x = sample->data_vector.x,
        .y = sample->data_vector.y,
        .z = sample->data_vector.z,
    };

    switch (sample->data_vector.sensor_id)
    {
    case VS_ID_ACCELEROMETER:
    case VS_ID_ACCELEROMETER_WAKEUP:
        out->type = IMU_SAMPLE_ACC;
        out->vector = vector;
        return true;
    case VS_ID_GYROSCOPE:
    case VS_ID_GYROSCOPE_WAKEUP:
        out->type = IMU_SAMPLE_GYR;
        out->vector = vector;
        return true;
    case VS_ID_MAGNETOMETER:
    case VS_ID_MAGNETOMETER_WAKEUP:
        out->type = IMU_SAMPLE_MAG;
        out->vector = vector;
        return true;
    default:
        return false;
    }
}

/**
 * @brief   Converts a raw quaternion FIFO sample into a public ImuSampleEvent_t,
 *          applying the mount-correction quaternion.
 * @param   sample Raw quaternion sample from the FIFO (Q14 fixed-point).
 * @param   out    Filled with IMU_SAMPLE_QUAT and the corrected orientation.
 */
static void handle_quaternion(bhy_data_generic_t *sample, ImuSampleEvent_t *out)
{
    /* Convert Q14 fixed-point integers to normalised floats in [-1, 1]. */
	float qw = sample->data_quaternion.w / BHI160_QUAT_SCALE;
	float qx = sample->data_quaternion.x / BHI160_QUAT_SCALE;
	float qy = sample->data_quaternion.y / BHI160_QUAT_SCALE;
	float qz = sample->data_quaternion.z / BHI160_QUAT_SCALE;

    out->type = IMU_SAMPLE_QUAT;
    /* Apply the mount-correction quaternion (identity by default). */
    out->quaternion.orientation = apply_correction((Quaternion){qw, qx, qy, qz});
    /* accuracy uses the same Q14 encoding; cast to uint8 gives a 0–1 range. */
    out->quaternion.accuracy = (uint8_t)(sample->data_quaternion.estimated_accuracy / BHI160_ACCURACY_SCALE);
}

/**
 * @brief   Logs the BHI160's own report of FIFO data loss (bytes 1/2 of this
 *          event carry a saturating lost-byte count, per the datasheet) and
 *          flags it so the caller can discard any in-progress batch.
 * @param   sample Raw meta event sample from the FIFO.
 * @param   out    Set to IMU_SAMPLE_OVERFLOW on a FIFO overflow event.
 */
static void handle_meta_event(bhy_data_generic_t *sample, ImuSampleEvent_t *out)
{
    if (sample->data_meta_event.event_number == BHY_META_EVENT_TYPE_FIFO_OVERFLOW)
    {
        NAQILOG_WARN("IMU: FIFO overflow reported by sensor (info bytes: %u, %u)\r",
                     sample->data_meta_event.sensor_type, sample->data_meta_event.event_specific);
        out->type = IMU_SAMPLE_OVERFLOW;
    }
}

/* ── public API ───────────────────────────────────────────────────────────── */

int8_t ImuItf_deviceInit(mxc_i2c_regs_t *i2c_inst)
{
    int8_t result;
    mxc_gpio_cfg_t irq_pin = {
        .port = IMU_INTERRUPT_PORT,
        .mask = IMU_INTERRUPT_PIN,
    };

    while (MXC_GPIO_InGet(irq_pin.port, irq_pin.mask) == 1);

    imuI2cInst = i2c_inst;

    if (IMU_driver_init((uint8_t *)&bhy1_firmware, i2c_inst)) {
        NAQILOG_ERROR("IMU firmware upload failed\r");
        return -1;
    }
    MXC_Delay(MXC_DELAY_MSEC(BHY_FIRMWARE_BOOT_DELAY_MS));

    while (MXC_GPIO_InGet(irq_pin.port, irq_pin.mask) == 1);

    result = bhy_mapping_matrix_set(i2c_inst, PHYSICAL_SENSOR_INDEX_ACC, imu_mapping);
    if (result)
        NAQILOG_ERROR("ACC mapping matrix failed: %d\r", result);

    result = bhy_mapping_matrix_set(i2c_inst, PHYSICAL_SENSOR_INDEX_MAG, mag_mapping);
    if (result)
        NAQILOG_ERROR("MAG mapping matrix failed: %d\r", result);

    result = bhy_mapping_matrix_set(i2c_inst, PHYSICAL_SENSOR_INDEX_GYRO, imu_mapping);
    if (result)
        NAQILOG_ERROR("GYRO mapping matrix failed: %d\r", result);

    result = IMU_enable_virtual_sensor(i2c_inst, VS_TYPE_ACCELEROMETER,
                                       VS_WAKEUP, SAMPLE_RATE, 0, VS_FLUSH_SINGLE, 1, 1);
    if (result)
        NAQILOG_ERROR("Enable accelerometer failed: %d\r", result);

    result = IMU_enable_virtual_sensor(i2c_inst, VS_TYPE_GYROSCOPE,
                                       VS_WAKEUP, SAMPLE_RATE, 0, VS_FLUSH_SINGLE, 0, 0);
    if (result)
        NAQILOG_ERROR("Enable gyroscope failed: %d\r", result);

    result = IMU_enable_virtual_sensor(i2c_inst, VS_TYPE_GEOMAGNETIC_FIELD,
                                       VS_WAKEUP, MAG_SAMPLE_RATE, MAG_REPORT_LATENCY_MS, VS_FLUSH_SINGLE, 0, 0);
    if (result)
        NAQILOG_ERROR("Enable magnetometer failed: %d\r", result);

    result = IMU_enable_virtual_sensor(i2c_inst, VS_TYPE_ROTATION_VECTOR,
                                       VS_WAKEUP, SAMPLE_RATE, 0, VS_FLUSH_SINGLE, 0, 0);
    if (result)
        NAQILOG_ERROR("Enable rotation vector failed: %d\r", result);

    return result;
}

/**
 * @brief   ISR fired on the BHI160 data-ready level interrupt. Masks the
 *          interrupt (re-armed once the FIFO is drained, see
 *          ImuItf_readNextSample()) and forwards to dataReadyCallback, which is
 *          responsible for any RTOS-specific notification (task-notify, etc).
 * @param   cbdata Unused (required by mxc_gpio_callback_fn).
 * @return  None.
 */
static void imu_gpio_isr(void *cbdata)
{
    (void)cbdata;

    /* Level-triggered: mask until ImuItf_readNextSample() drains the FIFO and re-arms it,
     * otherwise it keeps re-firing and starves the FreeRTOS context switch (PendSV). */
    MXC_GPIO_DisableInt(IMU_INTERRUPT_PORT, IMU_INTERRUPT_PIN);

    if (dataReadyCallback != NULL) {
        dataReadyCallback();
    }
}

void ImuItf_enableDataReadyInterrupt(ImuInterruptCallback_t callback)
{
    mxc_gpio_cfg_t irq_pin = {
        .port = IMU_INTERRUPT_PORT,
        .mask = IMU_INTERRUPT_PIN,
    };

    dataReadyCallback = callback;

    MXC_GPIO_IntConfig(&irq_pin, MXC_GPIO_INT_HIGH);
    MXC_GPIO_RegisterCallback(&irq_pin, imu_gpio_isr, NULL);
    MXC_GPIO_EnableInt(irq_pin.port, irq_pin.mask);

    /* Priority must be >= configMAX_SYSCALL_INTERRUPT_PRIORITY for the
     * callback to safely call FreeRTOS ISR-safe APIs — same priority as
     * UART0 in Communication.c. */
    NVIC_SetPriority(MXC_GPIO_GET_IRQ(MXC_GPIO_GET_IDX(irq_pin.port)), 5);
    NVIC_EnableIRQ(MXC_GPIO_GET_IRQ(MXC_GPIO_GET_IDX(irq_pin.port)));
}

/* Drains fresh FIFO bytes over I2C (bytes-remaining, then exactly that many
 * bytes) whenever the cursor's backlog has been fully parsed, then decodes
 * the next raw sample from it. */
static int8_t readNextRawSample(mxc_i2c_regs_t *i2c_inst, bhy_data_generic_t *out,
                                 bhy_data_type_t *out_type, bool *available)
{
    static uint8_t fifo_raw_buffer[BHY_FIFO_DATA_BUFFER];
    static FifoCursor_t fifo_cursor = {NULL, 0};

    *available = false;

    if (fifo_cursor.remaining == 0)
    {
        uint16_t bytes_remaining;

        if (IMU_read_bytes_remaining(i2c_inst, &bytes_remaining))
        {
            return BHY_COMM_RES;
        }

        if (bytes_remaining == 0)
        {
            return BHY_SUCCESS;
        }

        if (bytes_remaining > sizeof(fifo_raw_buffer))
        {
            NAQILOG_WARN("IMU: FIFO (%u bytes) exceeds host buffer, aborting transfer\r", bytes_remaining);
            IMU_abort_fifo_transfer(i2c_inst);
            return BHY_OUT_OF_RANGE;
        }

        if (IMU_read_fifo(i2c_inst, fifo_raw_buffer, bytes_remaining))
        {
            IMU_abort_fifo_transfer(i2c_inst);
            return BHY_DATA_LOST;
        }

        fifo_cursor.ptr = fifo_raw_buffer;
        fifo_cursor.remaining = bytes_remaining;
    }

    if (IMU_parse_next_raw_sample(&fifo_cursor, out, out_type))
    {
        /* lost sync — discard the rest of this backlog and resync on the next drain */
        fifo_cursor.remaining = 0;
        return BHY_DATA_LOST;
    }

    *available = true;
    return BHY_SUCCESS;
}

bool ImuItf_readNextSample(ImuSampleEvent_t *out)
{
    bhy_data_generic_t raw;
    bhy_data_type_t type;
    bool available;
    static uint8_t fifoOverflowRetries = 0;
    int8_t status = readNextRawSample(imuI2cInst, &raw, &type, &available);

    if (status == BHY_DATA_LOST || status == BHY_OUT_OF_RANGE)
    {
        if (++fifoOverflowRetries >= IMU_FIFO_OVERFLOW_RETRY_LIMIT)
        {
            NAQILOG_ERROR("IMU: FIFO overflow recovery failed after %d retries, giving up\r",
                          IMU_FIFO_OVERFLOW_RETRY_LIMIT);
            fifoOverflowRetries = 0;
            MXC_GPIO_EnableInt(IMU_INTERRUPT_PORT, IMU_INTERRUPT_PIN);
            return false;
        }

        NAQILOG_WARN("IMU: FIFO data loss, discarding in-progress batch\r");
        out->type = IMU_SAMPLE_OVERFLOW;
        return true;
    }

    fifoOverflowRetries = 0;

    if (status != BHY_SUCCESS)
    {
        /* plain comm error — nothing was actually lost, retry on the next interrupt */
        NAQILOG_WARN("IMU: FIFO read failed\r");
        MXC_GPIO_EnableInt(IMU_INTERRUPT_PORT, IMU_INTERRUPT_PIN);
        return false;
    }

    if (!available)
    {
        /* FIFO drained: re-arm the level-triggered interrupt (masked by imu_gpio_isr()). */
        MXC_GPIO_EnableInt(IMU_INTERRUPT_PORT, IMU_INTERRUPT_PIN);
        return false;
    }

    out->type = IMU_SAMPLE_NONE;

    switch (type)
    {
    case BHY_DATA_TYPE_VECTOR:
        handle_vector(&raw, out);
        break;
    case BHY_DATA_TYPE_QUATERNION:
        handle_quaternion(&raw, out);
        break;
    case BHY_DATA_TYPE_META_EVENT:
        handle_meta_event(&raw, out);
        break;
    default:
        break;
    }

    return true;
}

