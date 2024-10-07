/*! ----------------------------------------------------------------------------
 * @file    deca_spi.c
 * @brief   SPI access functions
 *
 * @attention
 *
 * Copyright 2015 (c) Decawave Ltd, Dublin, Ireland.
 *
 * All rights reserved.
 *
 * @author Decawave
 */

#include <deca_port.h>
#include "deca_spi.h"
#include "deca_device_api.h"
#include "stm32h7xx_hal_def.h"
#include "main.h"

extern  SPI_HandleTypeDef hspi6;    /*clocked from 72MHz*/

/****************************************************************************//**
 *
 *                              DW1000 SPI section
 *
 *******************************************************************************/
/*! ------------------------------------------------------------------------------------------------------------------
 * Function: openspi()
 * Low level abstract function to open and initialise access to the SPI device.
 * returns 0 for success, or -1 for error
 */
int openspi(/*SPI_TypeDef* SPIx*/)
{
    return 0;
} // end openspi()

/*! ------------------------------------------------------------------------------------------------------------------
 * Function: closespi()
 *
 * Low level abstract function to close the the SPI device.
 * returns 0 for success, or -1 for error
 */
int closespi(void)
{
    return 0;
} // end closespi()

/*! ------------------------------------------------------------------------------------------------------------------
 * Function: writetospi()
 *
 * Low level abstract function to write to the SPI
 * Takes two separate byte buffers for write header and write data
 * returns 0 for success
 */
#pragma GCC optimize ("O3")
int writetospi(uint16_t headerLength,
               const    uint8_t *headerBuffer,
               uint32_t bodyLength,
               const    uint8_t *bodyBuffer,
			   UWBPortTypeDef *pports)
{
    decaIrqStatus_t  stat ;
    stat = decamutexon(pports) ;

    while (HAL_SPI_GetState(pports->hspi) != HAL_SPI_STATE_READY);

    HAL_GPIO_WritePin(pports->spi_csn_port, pports->spi_csn_pin, GPIO_PIN_RESET); /**< Put chip select line low */

    HAL_SPI_Transmit(pports->hspi, (uint8_t *)&headerBuffer[0], headerLength, HAL_MAX_DELAY);    /* Send header in polling mode */
    HAL_SPI_Transmit(pports->hspi, (uint8_t *)&bodyBuffer[0], bodyLength, HAL_MAX_DELAY);        /* Send data in polling mode */

    HAL_GPIO_WritePin(pports->spi_csn_port, pports->spi_csn_pin, GPIO_PIN_SET); /**< Put chip select line high */

    decamutexoff(stat, pports);

    return 0;
} // end writetospi()


/*! ------------------------------------------------------------------------------------------------------------------
 * Function: readfromspi()
 *
 * Low level abstract function to read from the SPI
 * Takes two separate byte buffers for write header and read data
 * returns the offset into read buffer where first byte of read data may be found,
 * or returns 0
 */
//#pragma GCC optimize ("O3")
int readfromspi(uint16_t headerLength,
                const uint8_t *headerBuffer,
                uint32_t readlength,
                uint8_t *readBuffer,
				UWBPortTypeDef *pports)
{
//    uint8_t zeros[1]={0x00};
    decaIrqStatus_t  stat ;
    stat = decamutexon(pports);

    /* Blocking: Check whether previous transfer has been finished */
    while (HAL_SPI_GetState(pports->hspi) != HAL_SPI_STATE_READY);

    HAL_GPIO_WritePin(pports->spi_csn_port, pports->spi_csn_pin, GPIO_PIN_RESET); /**< Put chip select line low */

    HAL_SPI_Transmit(pports->hspi, (uint8_t *)headerBuffer, headerLength, HAL_MAX_DELAY);
    /* Send header */
//    for(i=0; i<headerLength; i++)
//    {
//        HAL_SPI_Transmit(&hspi6, (uint8_t *)&headerBuffer[i], 1, HAL_MAX_DELAY);//No timeout
//    }

    HAL_SPI_Receive(pports->hspi, (uint8_t *)readBuffer, readlength, HAL_MAX_DELAY);
    /* for the data buffer use LL functions directly as the HAL SPI read function
     * has issue reading single bytes */
//    while(readlength > 0)
//    {
//    	readlength--;
////        while(__HAL_SPI_GET_FLAG(&hspi6, SPI_FLAG_TXE) == RESET)
////        {
////        }
////    	HAL_SPI_Transmit(&hspi6,(uint8_t *)zeros,1,100);
//        /* Wait until TXE flag is set to send data */
////        while(__HAL_SPI_GET_FLAG(&hspi6, SPI_FLAG_TXE) == RESET)
////        {
////        }
////        hspi6.Instance->TXDR = 0;
//        /* set output to 0 (MOSI), this is necessary for
//        e.g. when waking up DW1000 from DEEPSLEEP via dwt_spicswakeup() function.
//        */
//        HAL_SPI_Receive(&hspi6, (uint8_t *)readBuffer, 1, 100);
//        /* Wait until RXNE flag is set to read data */
////        while(__HAL_SPI_GET_FLAG(&hspi6, SPI_FLAG_RXNE) == RESET)
////        {
////        }
//        readBuffer++;
////        (*readBuffer++) = hspi6.Instance->RXDR;  //copy data read form (MISO)
//    }
//    HAL_SPI_TransmitReceive(&hspi6,(uint8_t *)headerBuffer,(uint8_t *)readBuffer,headerLength,headerLength*100);

    HAL_GPIO_WritePin(pports->spi_csn_port, pports->spi_csn_pin, GPIO_PIN_SET); /**< Put chip select line high */

    decamutexoff(stat, pports);

    return 0;
} // end readfromspi()

/****************************************************************************//**
 *
 *                              END OF DW1000 SPI section
 *
 *******************************************************************************/

