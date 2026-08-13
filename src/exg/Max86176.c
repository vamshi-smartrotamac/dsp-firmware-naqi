/**************************************************************************
 * Copyright (c) 2026 Naqi Logix Inc. All Rights Reserved.
 *
 * File: Max86176.c
 *
 * This file is part of the proprietary firmware and is protected by
 * applicable copyright laws and international treaties. Unauthorized use,
 * reproduction, or distribution of this file or its contents is strictly
 * prohibited. For permissions, contact info @ naqilogix.com.
 **************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "Max86176.h"
#include <stddef.h>
#include "SPI_Driver.h"
#include "logger.h"
#include "mxc_delay.h"

/* Private define ------------------------------------------------------------*/
#define EXG_TAG_MASK                    0xFC
#define EXG_TAG_AUTO                    0xC0
#define EXG_TAG_LEADOFF_I               0xE0
#define EXG_TAG_LEADOFF_Q               0xE4

#define MAX86176_FIFO_OVF_COUNT_MASK    0x7F
#define MAX86176_FIFO_COUNT_MSB_MASK    0x80
#define MAX86176_SAMPLE_MSB_MASK        0x03
#define MAX86176_SAMPLE_SIGN_BIT        (1UL << 17)  // sign bit of the 18-bit signed sample
#define MAX86176_SAMPLE_SIZE            (3)

#define MAX86176_LEADOFF_SIGN_BIT       (1UL << 11)  // sign bit of the 12-bit signed lead-off sample
#define MAX86176_LEADOFF_INIT_MAGNITUDE (2047)
#define MAX86176_STATUS6_AC_LOFF_MASK   0x20

#define MAX86176_PARTID_EXPECTED        (0x39)

#define MAX86176_ECG_ADC_MAX_COUNT      (1UL << 17)  // 18-bit signed ADC -> 2^17 full-scale magnitude
#define MAX86176_ECG_VREF_uV            (1000000UL)  // 1000 mV reference

#define MAX86176_ECG_PGA_GAIN_FIELD     (0x0)  // ECG_PGA_GAIN[2:0], bits[6:4] -> see pgaGain table
#define MAX86176_ECG_INA_GAIN_FIELD     (0x0)  // ECG_INA_GAIN[1:0], bits[1:0] -> see inaGain table

/* Register addresses ---------------------------------------------------------*/
// PART ID //
#define MAX86176_PARTID_ADDRESS         0xFF

// ECG SETUP REGISTERS //
#define MAX86176_ECG_CONFIG1            0x90
#define MAX86176_ECG_CONFIG2            0x91
#define MAX86176_ECG_CONFIG3            0x92

// ECG LEAD DETECT REGISTERS //
#define MAX86176_LEADDETECT_CONFIG1     0x93
#define MAX86176_LEADDETECT_CONFIG2     0x94
#define MAX86176_AC_LEADDETECT_WAVEFORM 0x95
#define MAX86176_DC_LEADDETECT_DAC      0x96
#define MAX86176_DC_LEADOFF_THRESHOLD   0x97
#define MAX86176_AC_LEADOFF_THRESHOLD1  0x98
#define MAX86176_AC_LEADOFF_THRESHOLD2  0x99
#define MAX86176_AC_LEADOFF_PGA_HPF     0x9A
#define MAX86176_AC_LEADOFF_CALIBRES    0x9B

// ECG LEAD BIAS //
#define MAX86176_LEADBIAS_CONFIG1       0x9E

// ECG CALIBRATION //
#define MAX86176_CAL_CONFIG1            0xA0
#define MAX86176_CAL_CONFIG2            0xA1
#define MAX86176_CAL_CONFIG3            0xA2

// ECG RLD AND CM AMPS //
#define MAX86176_RLD_CONFIG1            0xA8
#define MAX86176_RLD_CONFIG2            0xA9

// STATUS REGISTERS //
#define MAX86176_STATUS1                0x00
#define MAX86176_STATUS2                0x01
#define MAX86176_STATUS3                0x02
#define MAX86176_STATUS4                0x03
#define MAX86176_STATUS5                0x04
#define MAX86176_STATUS6                0x05

// FIFO REGISTERS //
#define MAX86176_FIFOWRITEPTR           0x08
#define MAX86176_FIFOREADPTR            0x09
#define MAX86176_FIFOCOUNTER1           0x0A
#define MAX86176_FIFOCOUNTER2           0x0B
#define MAX86176_FIFODATA               0x0C
#define MAX86176_FIFOCONFIG1            0x0D
#define MAX86176_FIFOCONFIG2            0x0E

// SYSTEM CONTROL REGISTERS //
#define MAX86176_SYSTEMCONFIG1          0x10
#define MAX86176_SYSTEMCONFIG2          0x11
#define MAX86176_SYSTEMCONFIG3          0x12
#define MAX86176_SYSTEMCONFIG4          0x13
#define MAX86176_PHOTODIODE_BIAS        0x14
#define MAX86176_PIN_FUNC_CONFIG        0x15
#define MAX86176_OUTPUT_PINCONFIG       0x16
#define MAX86176_I2C_BROADCAST_ADDR     0x17

// PLL REGISTERS //
#define MAX86176_PLL_CONFIG1            0x18
#define MAX86176_PLL_CONFIG2            0x19
#define MAX86176_PLL_CONFIG3            0x1A

// PPG FRAME RATE CLOCK //
#define MAX86176_FR_CLOCKFREQSEL        0x1C
#define MAX86176_FR_CLOCKDIV_MSB        0x1D
#define MAX86176_FR_CLOCKDIV_LSB        0x1E

// PPG MEASUREMENT1 SETUP //
#define MAX86176_MEAS1_SELECT           0x20
#define MAX86176_MEAS1_CONFIG1          0x21
#define MAX86176_MEAS1_CONFIG2          0x22
#define MAX86176_MEAS1_CONFIG3          0x23
#define MAX86176_MEAS1_CONFIG4          0x24
#define MAX86176_MEAS1_LEDA_CURRENT     0x25
#define MAX86176_MEAS1_LEDB_CURRENT     0x26

// PPG MEASUREMENT2 SETUP //
#define MAX86176_MEAS2_SELECT           0x28
#define MAX86176_MEAS2_CONFIG1          0x29
#define MAX86176_MEAS2_CONFIG2          0x2A
#define MAX86176_MEAS2_CONFIG3          0x2B
#define MAX86176_MEAS2_CONFIG4          0x2C
#define MAX86176_MEAS2_LEDA_CURRENT     0x2D
#define MAX86176_MEAS2_LEDB_CURRENT     0x2E

// PPG MEASUREMENT3 SETUP //
#define MAX86176_MEAS3_SELECT           0x30
#define MAX86176_MEAS3_CONFIG1          0x31
#define MAX86176_MEAS3_CONFIG2          0x32
#define MAX86176_MEAS3_CONFIG3          0x33
#define MAX86176_MEAS3_CONFIG4          0x34
#define MAX86176_MEAS3_LEDA_CURRENT     0x35
#define MAX86176_MEAS3_LEDB_CURRENT     0x36

// PPG MEASUREMENT4 SETUP //
#define MAX86176_MEAS4_SELECT           0x38
#define MAX86176_MEAS4_CONFIG1          0x39
#define MAX86176_MEAS4_CONFIG2          0x3A
#define MAX86176_MEAS4_CONFIG3          0x3B
#define MAX86176_MEAS4_CONFIG4          0x3C
#define MAX86176_MEAS4_LEDA_CURRENT     0x3D
#define MAX86176_MEAS4_LEDB_CURRENT     0x3E

// PPG MEASUREMENT5 SETUP //
#define MAX86176_MEAS5_SELECT           0x40
#define MAX86176_MEAS5_CONFIG1          0x41
#define MAX86176_MEAS5_CONFIG2          0x42
#define MAX86176_MEAS5_CONFIG3          0x43
#define MAX86176_MEAS5_CONFIG4          0x44
#define MAX86176_MEAS5_LEDA_CURRENT     0x45
#define MAX86176_MEAS5_LEDB_CURRENT     0x46

// PPG MEASUREMENT6 SETUP //
#define MAX86176_MEAS6_SELECT           0x48
#define MAX86176_MEAS6_CONFIG1          0x49
#define MAX86176_MEAS6_CONFIG2          0x4A
#define MAX86176_MEAS6_CONFIG3          0x4B
#define MAX86176_MEAS6_CONFIG4          0x4C
#define MAX86176_MEAS6_LEDA_CURRENT     0x4D
#define MAX86176_MEAS6_LEDB_CURRENT     0x4E

// PPG MEASUREMENT7 SETUP //
#define MAX86176_MEAS7_SELECT           0x50
#define MAX86176_MEAS7_CONFIG1          0x51
#define MAX86176_MEAS7_CONFIG2          0x52
#define MAX86176_MEAS7_CONFIG3          0x53
#define MAX86176_MEAS7_CONFIG4          0x54
#define MAX86176_MEAS7_LEDA_CURRENT     0x55
#define MAX86176_MEAS7_LEDB_CURRENT     0x56

// PPG MEASUREMENT8 SETUP //
#define MAX86176_MEAS8_SELECT           0x58
#define MAX86176_MEAS8_CONFIG1          0x59
#define MAX86176_MEAS8_CONFIG2          0x5A
#define MAX86176_MEAS8_CONFIG3          0x5B
#define MAX86176_MEAS8_CONFIG4          0x5C
#define MAX86176_MEAS8_LEDA_CURRENT     0x5D
#define MAX86176_MEAS8_LEDB_CURRENT     0x5E

// PPG MEASUREMENT9 SETUP //
#define MAX86176_MEAS9_SELECT           0x60
#define MAX86176_MEAS9_CONFIG1          0x61
#define MAX86176_MEAS9_CONFIG2          0x62
#define MAX86176_MEAS9_CONFIG3          0x63
#define MAX86176_MEAS9_CONFIG4          0x64
#define MAX86176_MEAS9_LEDA_CURRENT     0x65
#define MAX86176_MEAS9_LEDB_CURRENT     0x66

// INTERRUPT ENABLE REGISTERS //
#define MAX86176_INTR1_ENABLE1          0x80
#define MAX86176_INTR1_ENABLE2          0x81
#define MAX86176_INTR1_ENABLE3          0x82
#define MAX86176_INTR1_ENABLE4          0x83
#define MAX86176_INTR1_ENABLE5          0x84
#define MAX86176_INTR1_ENABLE6          0x85
#define MAX86176_INTR2_ENABLE1          0x86
#define MAX86176_INTR2_ENABLE2          0x87
#define MAX86176_INTR2_ENABLE3          0x88
#define MAX86176_INTR2_ENABLE4          0x89
#define MAX86176_INTR2_ENABLE5          0x8A
#define MAX86176_INTR2_ENABLE6          0x8B

/* Private variables ---------------------------------------------------------*/
static mxc_spi_regs_t *spiHandle = NULL;

static int32_t lastLeadOffI = MAX86176_LEADOFF_INIT_MAGNITUDE;
static int32_t lastLeadOffQ = MAX86176_LEADOFF_INIT_MAGNITUDE;
static uint8_t lastLeadOffStatus = 1;  // assume "lead off" until the first AC_LOFF read

// Decode tables for the fields above
static const uint8_t pgaGain[8] = {1, 2, 4, 8, 0, 0, 0, 16};
static const uint8_t inaGain[4] = {20, 30, 40, 60};

static int readReg(uint8_t reg, uint8_t *value)
{
    return SPI_Read_Multibyte(spiHandle, reg, value, 1);
}

static int writeReg(uint8_t reg, uint8_t value)
{
    int status = SPI_Write(spiHandle, reg, value);
    if (status == E_SUCCESS)
    {
        MXC_Delay(100);
    }
    return status;
}

int8_t MAX86176_configureACLeadoff(void)
{
    uint8_t val;

    if (readReg(MAX86176_LEADDETECT_CONFIG1, &val) != E_SUCCESS)
    {
        return -1;
    }
    // DAC_STIM_MODE = 0 -> stimulus continuous between ADC conversions
    val &= ~(0x3 << 5 | 1 << 4);
    // EN_LOFF_DET = 2 -> AC lead-off enabled, cal resistor disconnected
    val |= 1 << 6;
    if (writeReg(MAX86176_LEADDETECT_CONFIG1, val) != E_SUCCESS)
    {
        return -1;
    }

    if (readReg(MAX86176_LEADDETECT_CONFIG2, &val) != E_SUCCESS)
    {
        return -1;
    }
    val &= ~(1 << 6 | 0x3 << 4 | 0x0F);
    // HI_CM_RES_EN = 1 -> high common-mode impedance enabled
    val |= 1 << 6;
    // LOFF_CG_MODE = 2 -> no common-mode feedback, no pullups
    val |= 1 << 5;
    // LOFF_IMAG = 5 -> 200nA
    val |= 0x5;
    if (writeReg(MAX86176_LEADDETECT_CONFIG2, val) != E_SUCCESS)
    {
        return -1;
    }

    // AC_LOFF_IWAVE = 1 -> sine wave ; AC_LOFF_FREQ_DIV = 3 -> 4096Hz, whole register
    if (writeReg(MAX86176_AC_LEADDETECT_WAVEFORM, 0x83) != E_SUCCESS)
    {
        return -1;
    }

    if (readReg(MAX86176_DC_LEADDETECT_DAC, &val) != E_SUCCESS)
    {
        return -1;
    }
    // AC_LOFF_CONV = 0 -> continuous conversions (~10sps)
    val &= ~(0x3 << 6);
    if (writeReg(MAX86176_DC_LEADDETECT_DAC, val) != E_SUCCESS)
    {
        return -1;
    }

    if (readReg(MAX86176_AC_LEADOFF_THRESHOLD1, &val) != E_SUCCESS)
    {
        return -1;
    }
    // AC_LOFF_CMP = 2 -> magnitude of Z used for threshold comparator
    val &= ~(0x3 << 6);
    val |= 1 << 7;
    if (writeReg(MAX86176_AC_LEADOFF_THRESHOLD1, val) != E_SUCCESS)
    {
        return -1;
    }

    // AC_LOFF_THRESH = 0x10 -> +-256 counts, whole register
    if (writeReg(MAX86176_AC_LEADOFF_THRESHOLD2, 0x10) != E_SUCCESS)
    {
        return -1;
    }

    if (readReg(MAX86176_AC_LEADOFF_PGA_HPF, &val) != E_SUCCESS)
    {
        return -1;
    }
    // AC_LOFF_UTIL_PGA_GAIN = 0 -> gain 3
    val &= ~(0x3 << 6 | 0x7);
    // AC_LOFF_HPF = 5 -> 3800Hz corner
    val |= 0x5;
    if (writeReg(MAX86176_AC_LEADOFF_PGA_HPF, val) != E_SUCCESS)
    {
        return -1;
    }

    return 0;
}

int8_t MAX86176_configureRLD(void)
{
    // RLD_EN=1, RLD_MODE=1 closed-loop, RLD_OOR_RAPID=1, EN_RLD_OOR=1,
    // ACTV_CM_P=1, ACTV_CM_N=1 (both required for closed-loop RLD), RLD_GAIN=01 -> 24.5V/V ; whole register
    if (writeReg(MAX86176_RLD_CONFIG1, 0xFD) != E_SUCCESS)
    {
        return -1;
    }

    // RLD_EXT_RES=0 internal resistor, SEL_VCM_IN=1 ECGP/ECGN, RLD_BW=0 low bandwidth,
    // BODY_BIAS_DAC=0 -> Vmid_ecg ; whole register
    if (writeReg(MAX86176_RLD_CONFIG2, 1 << 6) != E_SUCCESS)
    {
        return -1;
    }

    return 0;
}

int8_t MAX86176_configurePLL(void)
{
    uint8_t val;

    // NDIV is the whole PLL_CONFIG3 register: 0x3F -> NDIV = 319
    if (writeReg(MAX86176_PLL_CONFIG3, 0x3F) != E_SUCCESS)
    {
        return -1;
    }

    if (readReg(MAX86176_PLL_CONFIG2, &val) != E_SUCCESS)
    {
        return -1;
    }
    // NDIV MSB, MDIV untouched
    val &= ~0x80;
    val |= 1 << 7;
    if (writeReg(MAX86176_PLL_CONFIG2, val) != E_SUCCESS)
    {
        return -1;
    }

    if (readReg(MAX86176_PLL_CONFIG1, &val) != E_SUCCESS)
    {
        return -1;
    }
    // PLL_EN: PLL disabled (internal oscillator used)
    val &= ~0x01;
    if (writeReg(MAX86176_PLL_CONFIG1, val) != E_SUCCESS)
    {
        return -1;
    }

    return 0;
}

int8_t MAX86176_configureECGChannel(void)
{
    uint8_t val;

    if (readReg(MAX86176_ECG_CONFIG1, &val) != E_SUCCESS)
    {
        return -1;
    }
    // ECG_DEC_RATE = 3 -> sample rate is 32768/128 = 256sps
    val &= ~0x07;
    val |= 0x3;
    if (writeReg(MAX86176_ECG_CONFIG1, val) != E_SUCCESS)
    {
        return -1;
    }

    if (readReg(MAX86176_ECG_CONFIG2, &val) != E_SUCCESS)
    {
        return -1;
    }
    val &= ~(0x7 << 4 | 0x3);
    val |= (MAX86176_ECG_PGA_GAIN_FIELD << 4) | MAX86176_ECG_INA_GAIN_FIELD;
    if (writeReg(MAX86176_ECG_CONFIG2, val) != E_SUCCESS)
    {
        return -1;
    }

    // EN_ECG_FAST_REC = 0 normal mode ; ECG_FAST_REC_THRESHOLD = 0x3F, whole register
    if (writeReg(MAX86176_ECG_CONFIG3, 0x3F) != E_SUCCESS)
    {
        return -1;
    }

    return 0;
}

uint32_t MAX86176_convertMicroVoltToADC(uint32_t microVolt)
{
    uint64_t temp;
    uint32_t totalGain = (uint32_t)pgaGain[MAX86176_ECG_PGA_GAIN_FIELD] * inaGain[MAX86176_ECG_INA_GAIN_FIELD];

    // ADC_Count = (uV * MaxCounts * TotalGain) / Vref_uV
    temp = (uint64_t)microVolt * MAX86176_ECG_ADC_MAX_COUNT * totalGain;

    temp /= MAX86176_ECG_VREF_uV;

    if (temp > MAX86176_ECG_ADC_MAX_COUNT)
    {
        temp = MAX86176_ECG_ADC_MAX_COUNT;
    }

    return (uint32_t)temp;
}

int8_t MAX86176_configureLeadBias(void)
{
    uint8_t val;

    if (readReg(MAX86176_LEADBIAS_CONFIG1, &val) != E_SUCCESS)
    {
        return -1;
    }
    // RBIAS_VALUE untouched, EN_RBIAS_P = 0, EN_RBIAS_N = 0, both disabled
    val &= ~(1 << 1 | 1);
    if (writeReg(MAX86176_LEADBIAS_CONFIG1, val) != E_SUCCESS)
    {
        return -1;
    }

    if (readReg(MAX86176_CAL_CONFIG3, &val) != E_SUCCESS)
    {
        return -1;
    }
    // OPEN_P = 0, OPEN_N = 0 -> ECGP/ECGN inputs connected to AFE
    val &= ~(1 << 7 | 1 << 6);
    if (writeReg(MAX86176_CAL_CONFIG3, val) != E_SUCCESS)
    {
        return -1;
    }

    return 0;
}

int8_t MAX86176_flushFifo(void)
{
    uint8_t val;

    if (readReg(MAX86176_FIFOCONFIG2, &val) != E_SUCCESS)
    {
        return -1;
    }
    // FLUSH_FIFO, self-clearing
    val |= 1 << 4;
    if (writeReg(MAX86176_FIFOCONFIG2, val) != E_SUCCESS)
    {
        return -1;
    }

    return 0;
}

/**
 * @brief Reads AFE device ID and checks if it matches the expected value.
 *
 */
static int readDeviceID(void)
{
    uint8_t part_id;

    // Read AFE Device ID
    if (SPI_Read_Multibyte(spiHandle, MAX86176_PARTID_ADDRESS, &part_id, 1) != E_SUCCESS)
    {
        return E_NO_RESPONSE;
    }

    if (part_id == MAX86176_PARTID_EXPECTED)
    {
        NAQILOG_INFO("The EEG device id is 0x%02X", part_id);
        return E_SUCCESS;
    }
    else
    {
        NAQILOG_ERROR("Error Reading ID (Read: 0x%02X)", part_id);
        return E_COMM_ERR;  // Device ID mismatch
    }
}

int8_t MAX86176_init(mxc_spi_regs_t *spi)
{
    if (spi)
    {
        spiHandle = spi;
    }
    else
    {
        NAQILOG_ERROR("Spi handle is NULL");
        return -1;
    }

    if (readDeviceID() != E_SUCCESS)
    {
        NAQILOG_ERROR("Device ID verification failed!");
        return -1;
    }
    else
    {
        NAQILOG_INFO("Communication with ExG sensor established");
    }

    return 0;
}

void MAX86176_startAcquisition(void)
{
    uint8_t val;

    if (spiHandle == NULL)
    {
        NAQILOG_ERROR("MAX86176_startAcquisition called before MAX86176_init");
        return;
    }

    if (readReg(MAX86176_ECG_CONFIG1, &val) != E_SUCCESS)
    {
        NAQILOG_ERROR("Failed to read ECG_CONFIG1 before starting acquisition");
        return;
    }
    // ECG_EN = 1
    val |= 1 << 7;
    if (writeReg(MAX86176_ECG_CONFIG1, val) != E_SUCCESS)
    {
        NAQILOG_ERROR("Failed to enable ECG acquisition");
    }
}

void MAX86176_stopAcquisition(void)
{
    uint8_t val;

    if (spiHandle == NULL)
    {
        NAQILOG_ERROR("MAX86176_stopAcquisition called before MAX86176_init");
        return;
    }

    if (readReg(MAX86176_ECG_CONFIG1, &val) != E_SUCCESS)
    {
        NAQILOG_ERROR("Failed to read ECG_CONFIG1 before stopping acquisition");
        return;
    }
    // ECG_EN = 0
    val &= ~(1 << 7);
    if (writeReg(MAX86176_ECG_CONFIG1, val) != E_SUCCESS)
    {
        NAQILOG_ERROR("Failed to disable ECG acquisition");
    }
}

int8_t MAX86176_readSample(int32_t *sample)
{
    uint8_t  fifoCounter1, fifoCounter2;
    uint8_t  overflowCount;
    uint16_t fifoSampleCount;
    uint8_t  dataBuffer[MAX86176_SAMPLE_SIZE];
    uint8_t  header, sampleMsb;
    uint32_t rawSample;

    if (spiHandle == NULL)
    {
        NAQILOG_ERROR("MAX86176_readSample called before MAX86176_init");
        return -1;
    }

    if (SPI_Read_Multibyte(spiHandle, MAX86176_FIFOCOUNTER1, &fifoCounter1, 1) != E_SUCCESS)
    {
        return -1;
    }

    if (SPI_Read_Multibyte(spiHandle, MAX86176_FIFOCOUNTER2, &fifoCounter2, 1) != E_SUCCESS)
    {
        return -1;
    }

    overflowCount = fifoCounter1 & MAX86176_FIFO_OVF_COUNT_MASK;
    // FIFOCOUNTER1's top bit is the count's extra high bit, only valid when no overflow occurred
    fifoSampleCount = overflowCount ? 256 : (((fifoCounter1 & MAX86176_FIFO_COUNT_MSB_MASK) << 1) | fifoCounter2);

    if (!fifoSampleCount)
    {
        return 0;
    }

    // Read a sample from AFE
    if (SPI_Read_Multibyte(spiHandle, MAX86176_FIFODATA, dataBuffer, sizeof(dataBuffer)) != E_SUCCESS)
    {
        return -1;
    }

    header = dataBuffer[0];

    if ((header & EXG_TAG_MASK) == EXG_TAG_LEADOFF_I || (header & EXG_TAG_MASK) == EXG_TAG_LEADOFF_Q)
    {
        uint32_t rawLeadOff = ((uint32_t)(dataBuffer[1] & 0x0F) << 8) | dataBuffer[2];
        int32_t  leadOffValue = (int32_t)((rawLeadOff ^ MAX86176_LEADOFF_SIGN_BIT) - MAX86176_LEADOFF_SIGN_BIT);

        // Cache lead off
        if ((header & EXG_TAG_MASK) == EXG_TAG_LEADOFF_I)
        {
            lastLeadOffI = leadOffValue;
        }
        else
        {
            uint8_t status6;

            lastLeadOffQ = leadOffValue;

            // Q always follows I for a given conversion, so AC_LOFF (reasserted ~488us
            // after the pair is pushed) is already settled by the time we read it here.
            // Reading STATUS6 clears AC_LOFF *and* every other bit in it (LON, DC_LOFF_*).
            // Do NOT add another STATUS6 read anywhere else: it would steal bits from this
            // read (or vice versa), silently dropping whichever status the other reader wanted.
            if (readReg(MAX86176_STATUS6, &status6) == E_SUCCESS)
            {
                lastLeadOffStatus = (status6 & MAX86176_STATUS6_AC_LOFF_MASK) ? 1 : 0;
            }
        }

        return 0;
    }

    // Only auto/normal-recovery frames (tag 0xC0) carry a valid ECG sample; manual recovery (0xC4) is ignored here
    if ((header & EXG_TAG_MASK) != EXG_TAG_AUTO)
    {
        return 0;
    }

    sampleMsb = header & MAX86176_SAMPLE_MSB_MASK;

    // 18-bit signed sample split across header + 2 bytes; sign-extend via unsigned XOR/subtract
    rawSample = ((uint32_t)sampleMsb << 16) | ((uint32_t)dataBuffer[1] << 8) | dataBuffer[2];
    *sample = (int32_t)((rawSample ^ MAX86176_SAMPLE_SIGN_BIT) - MAX86176_SAMPLE_SIGN_BIT);

    return 1;
}

void MAX86176_getLeadOffData(int32_t *iValue, int32_t *qValue, uint8_t *status)
{
    *iValue = lastLeadOffI;
    *qValue = lastLeadOffQ;
    *status = lastLeadOffStatus;
}
