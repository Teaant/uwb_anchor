/*! ----------------------------------------------------------------------------
 * @file    port.c
 * @brief   HW specific definitions and functions for portability
 *
 * @attention
 *
 * Copyright 2016 (c) Decawave Ltd, Dublin, Ireland.
 *
 * All rights reserved.
 *
 * @author Decawave
 */

#include <sys/types.h>

#include "deca_port.h"
#include "deca_device_api.h"
#include "stm32h7xx_hal_conf.h"
#include "main.h"
#include "uwb.h"
/****************************************************************************//**
 *
 *                              APP global variables
 *
 *******************************************************************************/
extern SPI_HandleTypeDef hspi6;
volatile extern UWBDef UWB;
/****************************************************************************//**
 *
 *                  Port private variables and function prototypes
 *
 *******************************************************************************/
static volatile uint32_t signalResetDone;

/****************************************************************************//**
 *
 *                              Time section
 *
 *******************************************************************************/

/* @fn    portGetTickCnt
 * @brief wrapper for to read a SysTickTimer, which is incremented with
 *        CLOCKS_PER_SEC frequency.
 *        The resolution of time32_incr is usually 1/1000 sec.
 * */
__INLINE uint32_t
portGetTickCnt(void)
{
    return HAL_GetTick();
}


/* @fn    usleep
 * @brief precise usleep() delay
 * */
#pragma GCC optimize ("O0")
void usleep(uint16_t usec)
{
    int i,j;
#pragma GCC ivdep
    for(i=0;i<usec;i++)
    {
#pragma GCC ivdep
        for(j=0;j<2;j++)
        {
            __NOP();
            __NOP();
        }
    }
}


/* @fn    Sleep
 * @brief Sleep delay in ms using SysTick timer
 * */
__INLINE void
Sleep(uint32_t x)
{
    HAL_Delay(x);
}

/****************************************************************************//**
 *
 *                              END OF Time section
 *
 *******************************************************************************/

/****************************************************************************//**
 *
 *                              Configuration section
 *
 *******************************************************************************/

/* @fn    peripherals_init
 * */
int peripherals_init (void)
{
    /* All has been initialized in the CubeMx code, see main.c */
    return 0;
}


/* @fn    spi_peripheral_init
 * */
void spi_peripheral_init()
{

    /* SPI's has been initialized in the CubeMx code, see main.c */

}



/**
  * @brief  Checks whether the specified EXTI line is enabled or not.
  * @param  EXTI_Line: specifies the EXTI line to check.
  *   This parameter can be:
  *     @arg EXTI_Linex: External interrupt line x where x(0..19)
  * @retval The "enable" state of EXTI_Line (SET or RESET).
  */
ITStatus EXTI_GetITEnStatus(uint32_t x)
{
    return ((NVIC->ISER[(((uint32_t)x) >> 5UL)] &\
            (uint32_t)(1UL << (((uint32_t)x) & 0x1FUL)) ) == (uint32_t)RESET)?(RESET):(SET);
}
/****************************************************************************//**
 *
 *                          End of configuration section
 *
 *******************************************************************************/

/****************************************************************************//**
 *
 *                          DW1000 port section
 *
 *******************************************************************************/

/* @fn      reset_DW1000
 * @brief   DW_RESET pin on DW1000 has 2 functions
 *          In general it is output, but it also can be used to reset the digital
 *          part of DW1000 by driving this pin low.
 *          Note, the DW_RESET pin should not be driven high externally.
 * */
void reset_DW1000(UWBPortTypeDef *pports)
{
//    GPIO_InitTypeDef    GPIO_InitStruct;

    // Enable GPIO used for DW1000 reset as open collector output
//    GPIO_InitStruct.Pin = DW_RESET_Pin;
//    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
//    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//    HAL_GPIO_Init(DW_RESET_GPIO_Port, &GPIO_InitStruct);

    //drive the RSTn pin low
    HAL_GPIO_WritePin(pports->rstn_port, pports->rstn_pin, GPIO_PIN_RESET);

    HAL_Delay(1);

    HAL_GPIO_WritePin(pports->rstn_port, pports->rstn_pin, GPIO_PIN_SET);
//    usleep(1);

    //put the pin back to output open-drain (not active)
//    setup_DW1000RSTnIRQ(0);



    Sleep(2);
}

/* @fn      setup_DW1000RSTnIRQ
 * @brief   setup the DW_RESET pin mode
 *          0 - output Open collector mode
 *          !0 - input mode with connected EXTI0 IRQ
 * */
void setup_DW1000RSTnIRQ(int enable)
{
//    GPIO_InitTypeDef GPIO_InitStruct;
//
//    if(enable)
//    {
//        // Enable GPIO used as DECA RESET for interrupt
//        GPIO_InitStruct.Pin = DW_RESET_Pin;
//        GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
//        GPIO_InitStruct.Pull = GPIO_NOPULL;
//        HAL_GPIO_Init(DW_RESET_GPIO_Port, &GPIO_InitStruct);
//
//        HAL_NVIC_EnableIRQ(EXTI0_IRQn);     //pin #0 -> EXTI #0
//        HAL_NVIC_SetPriority(EXTI0_IRQn, 5, 0);
//    }
//    else
//    {
//        HAL_NVIC_DisableIRQ(EXTI0_IRQn);    //pin #0 -> EXTI #0
//
//        //put the pin back to tri-state ... as
//        //output open-drain (not active)
//        GPIO_InitStruct.Pin = DW_RESET_Pin;
//        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
//        GPIO_InitStruct.Pull = GPIO_NOPULL;
//        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
//        HAL_GPIO_Init(DW_RESET_GPIO_Port, &GPIO_InitStruct);
//        HAL_GPIO_WritePin(DW_RESET_GPIO_Port, DW_RESET_Pin, GPIO_PIN_SET);
//    }
}


/* @fn      port_is_boot1_low
 * @brief   check the BOOT1 pin status.
 * @return  1 if ON and 0 for OFF
 * */
int port_is_boot1_low(void)
{
    return ((GPIO_ReadInputDataBit(TA_BOOT1_GPIO, TA_BOOT1))?(0):(1));
}

/* @fn      port_is_boot1_low
 * @brief   check the BOOT1 pin status.
 * @return  1 if ON and 0 for OFF
 * */
int port_is_boot1_on(uint16_t x)
{
    return ((GPIO_ReadInputDataBit(TA_BOOT1_GPIO, TA_BOOT1))?(0):(1));
}

/* @fn      port_is_switch_on
 * @brief   check the switch status.
 *          when switch (S1) is 'on' the pin is low
 * @return  1 if ON and 0 for OFF
 * */
int port_is_switch_on(uint16_t GPIOpin)
{
    return ((GPIO_ReadInputDataBit(TA_SW1_GPIO, GPIOpin))?(0):(1));
}

/* @fn      port_wakeup_dw1000
 * @brief   "slow" waking up of DW1000 using DW_CS only
 * */
void port_wakeup_dw1000(UWBPortTypeDef *pports)
{
    HAL_GPIO_WritePin(pports->spi_csn_port, pports->spi_csn_pin, GPIO_PIN_RESET);
    Sleep(1);
    HAL_GPIO_WritePin(pports->spi_csn_port, pports->spi_csn_pin, GPIO_PIN_SET);
    Sleep(7);                       //wait 7ms for DW1000 XTAL to stabilise
}

/* @fn      port_wakeup_dw1000_fast
 * @brief   waking up of DW1000 using DW_CS and DW_RESET pins.
 *          The DW_RESET signalling that the DW1000 is in the INIT state.
 *          the total fast wakeup takes ~2.2ms and depends on crystal startup time
 * */
void port_wakeup_dw1000_fast(UWBPortTypeDef *pports)
{
    #define WAKEUP_TMR_MS   (10)

    uint32_t x = 0;
    uint32_t timestamp = HAL_GetTick(); //protection

    setup_DW1000RSTnIRQ(0);         //disable RSTn IRQ
    signalResetDone = 0;            //signalResetDone connected to RST_PIN_IRQ
    setup_DW1000RSTnIRQ(1);         //enable RSTn IRQ
    HAL_GPIO_WritePin(pports->spi_csn_port, pports->spi_csn_pin, GPIO_PIN_RESET);  //CS low

    //need to poll to check when the DW1000 is in the IDLE, the CPLL interrupt is not reliable
    //when RSTn goes high the DW1000 is in INIT, it will enter IDLE after PLL lock (in 5 us)
    while((signalResetDone == 0) && \
          ((HAL_GetTick() - timestamp) < WAKEUP_TMR_MS))
    {
        x++;     //when DW1000 will switch to an IDLE state RSTn pin will high
    }
    setup_DW1000RSTnIRQ(0);         //disable RSTn IRQ
    HAL_GPIO_WritePin(pports->spi_csn_port, pports->spi_csn_pin, GPIO_PIN_SET);    //CS high

    //it takes ~35us in total for the DW1000 to lock the PLL, download AON and go to IDLE state
    usleep(35);
}



/* @fn      port_set_dw1000_slowrate
 * @brief   set 2.25MHz
 *          note: hspi6 is clocked from 72MHz
 * */
void port_set_dw1000_slowrate(UWBPortTypeDef *pports)
{
    pports->hspi->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
    HAL_SPI_Init(pports->hspi);
}

/* @fn      port_set_dw1000_fastrate
 * @brief   set 18MHz
 *          note: hspi6 is clocked from 72MHz
 * */
void port_set_dw1000_fastrate(UWBPortTypeDef *pports)
{
	pports->hspi->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
    HAL_SPI_Init(pports->hspi);
}

/****************************************************************************//**
 *
 *                          End APP port section
 *
 *******************************************************************************/



/****************************************************************************//**
 *
 *                              IRQ section
 *
 *******************************************************************************/

/* @fn      HAL_GPIO_EXTI_Callback
 * @brief   IRQ HAL call-back for all EXTI configured lines
 *          i.e. DW_RESET_Pin and DW_IRQn_Pin
 * */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	for(int i=0;i<DWT_NUM_DW_DEV;i++)
	{
		if(GPIO_Pin == UWB.ports[i].irq_pin)
		{
			dwt_setlocaldataptr(i);
			process_deca_irq(&UWB.ports[i]);
		}
	}
//    if (GPIO_Pin == DW_RESET_Pin)
//    {
//        signalResetDone = 1;
//    }
//    else if (GPIO_Pin == DW_IRQn_Pin)
//    {
//        process_deca_irq();
//    }
//    else
//    {
//    }
}

/* @fn      process_deca_irq
 * @brief   main call-back for processing of DW1000 IRQ
 *          it re-enters the IRQ routing and processes all events.
 *          After processing of all events, DW1000 will clear the IRQ line.
 * */
__INLINE void process_deca_irq(UWBPortTypeDef *pports)
{
    while(port_CheckEXT_IRQ(pports) != 0)
    {

        port_deca_isr(pports);

    } //while DW1000 IRQ line active
}


/* @fn      port_DisableEXT_IRQ
 * @brief   wrapper to disable DW_IRQ pin IRQ
 *          in current implementation it disables all IRQ from lines 5:9
 * */
__INLINE void port_DisableEXT_IRQ(UWBPortTypeDef *pports)
{
    NVIC_DisableIRQ(pports->exti_line);
}

/* @fn      port_EnableEXT_IRQ
 * @brief   wrapper to enable DW_IRQ pin IRQ
 *          in current implementation it enables all IRQ from lines 5:9
 * */
__INLINE void port_EnableEXT_IRQ(UWBPortTypeDef *pports)
{
    NVIC_EnableIRQ(pports->exti_line);
}


/* @fn      port_GetEXT_IRQStatus
 * @brief   wrapper to read a DW_IRQ pin IRQ status
 * */
__INLINE uint32_t port_GetEXT_IRQStatus(UWBPortTypeDef *pports)
{
    return EXTI_GetITEnStatus(pports->exti_line);
}


/* @fn      port_CheckEXT_IRQ
 * @brief   wrapper to read DW_IRQ input pin state
 * */
__INLINE uint32_t port_CheckEXT_IRQ(UWBPortTypeDef *pports)
{
    return HAL_GPIO_ReadPin(pports->irq_port, pports->irq_pin);
}


/****************************************************************************//**
 *
 *                              END OF IRQ section
 *
 *******************************************************************************/

/* DW1000 IRQ handler definition. */
port_deca_isr_t port_deca_isr = NULL;

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn port_set_deca_isr()
 *
 * @brief This function is used to install the handling function for DW1000 IRQ.
 *
 * NOTE:
 *   - As EXTI9_5_IRQHandler does not check that port_deca_isr is not null, the user application must ensure that a
 *     proper handler is set by calling this function before any DW1000 IRQ occurs!
 *   - This function makes sure the DW1000 IRQ line is deactivated while the handler is installed.
 *
 * @param deca_isr function pointer to DW1000 interrupt handler to install
 *
 * @return none
 */
void port_set_deca_isr(port_deca_isr_t deca_isr)
{
//    /* Check DW1000 IRQ activation status. */
//    ITStatus en = port_GetEXT_IRQStatus();
//
//    /* If needed, deactivate DW1000 IRQ during the installation of the new handler. */
//    if (en)
//    {
//        port_DisableEXT_IRQ();
//    }
    port_deca_isr = deca_isr;
//    if (en)
//    {
//        port_EnableEXT_IRQ();
//    }
}


/****************************************************************************//**
 *
 *******************************************************************************/

