/*
 * SPI_Driver.c
 *
 *  Created on: 27-Jan-2026
 *     Author: Naqi
 */

#include "board.h"

#include "SPI_Driver.h"
#include "logger.h"

#define REG_WRITE_CMD (0x7F)
#define REG_READ_CMD  (0x80)

#define WRITE_EN      (0x00)
#define READ_EN       (0xFF)

#define SPI_SPEED     5000000 /*SPI Clock set to 5MHz */

/*
 *---------------------------------------------------------------------------------------------
 * Function:      SPI_Initialization
 * Description:   This function is Initalization of SPI Periperal
 * Parameters:    [In1]mxc_spi_regs_t *spi     (SPI Instance)
 * return         Status of SPI Initialization
 *  ---------------------------------------------------------------------------------------------
 */
int SPI_Initialization(mxc_spi_regs_t *spi)
{
    mxc_spi_pins_t spi_pins;

    int ret = 0;
    int masterMode = 1;
    int quadModeUsed = 0;
    int numSlaves = 1;
    int ssPolarity = 0;

    spi_pins.clock = TRUE;
    spi_pins.miso = TRUE;
    spi_pins.mosi = TRUE;
    spi_pins.sdio2 = FALSE;
    spi_pins.sdio3 = FALSE;
#if defined(EVALBOARD)
    spi_pins.ss0 = FALSE;
    spi_pins.ss1 = TRUE;  // ss1 for the Evaluation hardware
    spi_pins.ss2 = FALSE;
#else
    spi_pins.ss0 = TRUE;  // ss0 for the P3 hardware
    spi_pins.ss1 = FALSE;
    spi_pins.ss2 = FALSE;
#endif
    ret = MXC_SPI_Init(spi, masterMode, quadModeUsed, numSlaves, ssPolarity, SPI_SPEED, spi_pins);
    if (ret)
    {
        return ret;
    }
    MXC_SPI_SetMode(spi, SPI_MODE_0);
    MXC_SPI_SetDataSize(spi, 8);
    MXC_SPI_SetWidth(spi, SPI_WIDTH_STANDARD);

    NVIC_SetPriority(SPI0_IRQn, 7);

    return ret;
}

/*
 *---------------------------------------------------------------------------------------------
 * Function:      SPI_Write
 * Description:   This function is to write data to given register
 * Parameters:    [In1]mxc_spi_regs_t *spi                (SPI Instance)
 * 				 [In2] uint8_t reg                		 (Register Address)
 * 				 [In3] uint8_t val                		 (Value)
 * return         Status of SPI Transaction
 *  ---------------------------------------------------------------------------------------------
 */

int SPI_Write(mxc_spi_regs_t *spi, uint8_t reg, uint8_t val)
{
    mxc_spi_req_t req;
    uint8_t       command[] = {reg, REG_WRITE_CMD, val};

    req.spi = spi;
    req.txData = command;
    req.rxData = 0;
    req.txLen = sizeof(command);
    req.rxLen = 0;
#if defined(EVALBOARD)
    req.ssIdx = 1;
#else
    req.ssIdx = 0;
#endif
    req.ssDeassert = 1;
    req.txCnt = 0;
    req.rxCnt = 0;
    req.completeCB = NULL;

    return MXC_SPI_MasterTransaction(&req);
}

/*
 *---------------------------------------------------------------------------------------------
 * Function:      SPI_Read_Multibyte
 * Description:   This function is to read data using SPI Peripheral
 * Parameters:    [In1]mxc_spi_regs_t *spi                (SPI Instance)
 * 				 [In2] uint8_t reg                		 (Register Address)
 * 				 [In3] uint8_t *val                		 (Value)
 * 				 [In4] uint8_t size                		 (size)
 * return         Status of SPI Transaction
 *  ---------------------------------------------------------------------------------------------
 */
int SPI_Read_Multibyte(mxc_spi_regs_t *spi, uint8_t reg, uint8_t *val, uint8_t size)
{
    const uint8_t tx_frame[] = {reg, REG_READ_CMD};
    mxc_spi_req_t req;

    // SPI Request
    req.spi = spi;
#if defined(EVALBOARD)
    req.ssIdx = 1;
#else
    req.ssIdx = 0;
#endif
    req.txCnt = 0;
    req.rxCnt = 0;

    // First part of frame: transmission
    req.txData = (uint8_t *)tx_frame;
    req.txLen = sizeof(tx_frame);
    req.rxLen = 0;
    // Set deassert to 0 so SS line remains selected after transmission
    req.ssDeassert = 0;
    if (MXC_SPI_MasterTransaction(&req) != E_SUCCESS)
    {
        return E_FAIL;
    }

    // Second part of frame: reception
    req.txData = NULL;
    req.rxData = (uint8_t *)val;
    req.rxLen = size;
    req.txLen = 0;
    req.ssDeassert = 1;
    if (MXC_SPI_MasterTransaction(&req) != E_SUCCESS)
    {
        return E_FAIL;
    }

    return E_SUCCESS;
}
