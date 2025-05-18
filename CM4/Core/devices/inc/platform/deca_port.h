/*! ----------------------------------------------------------------------------
 * @file    port.h
 * @brief   HW specific definitions and functions for portability
 *
 * @attention
 *
 * Copyright 2015 (c) Decawave Ltd, Dublin, Ireland.
 *
 * All rights reserved.
 *
 * @author Decawave
 */


#ifndef PORT_H_
#define PORT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <string.h>
#include "compiler.h"

#include "stm32h7xx.h"
#include "stm32h7xx_hal.h"

typedef enum{

	Ds_Twr_Port = 0,
	PDoA_Port,

}Port_Type_t;

typedef struct
{
	SPI_HandleTypeDef * hspi;
	GPIO_TypeDef * spi_csn_port;
	uint16_t spi_csn_pin;
	GPIO_TypeDef * wakeup_port;
	uint16_t wakeup_pin;
	GPIO_TypeDef * rstn_port;
	uint16_t rstn_pin;
	GPIO_TypeDef * irq_port;
	uint16_t irq_pin;
	uint32_t exti_line;
	int8_t avalible;
	Port_Type_t port_type;
	//SPI 初始化
	void (*port_init)(void);

} UWBPortTypeDef;



/* DW1000 IRQ (EXTI9_5_IRQ) handler type. */
typedef void (*port_deca_isr_t)(UWBPortTypeDef *pports);

/* DW1000 IRQ handler declaration. */
extern port_deca_isr_t port_deca_isr;

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
void port_set_deca_isr(port_deca_isr_t deca_isr);

#define BUFFLEN     (64) //(4096+128)

#define BUF_SIZE    (64)

#define USB_SUPPORT

typedef struct
{
    uint16_t        usblen;                 /**< for RX from USB */
    uint8_t         usbbuf[BUF_SIZE*3];     /**< for RX from USB */
}__packed app_t;


extern app_t    app;


/*****************************************************************************************************************//*
**/

 /****************************************************************************//**
  *
  *                                 Types definitions
  *
  *******************************************************************************/
typedef uint64_t        uint64 ;

typedef int64_t         int64 ;


#ifndef FALSE
#define FALSE               0
#endif

#ifndef TRUE
#define TRUE                1
#endif

typedef enum
{
    LED_PC6, //LED5
    LED_PC7, //LED6
    LED_PC8, //LED7
    LED_PC9, //LED8
    LED_ALL,
    LEDn
} led_t;

/****************************************************************************//**
 *
 *                              MACRO
 *
 *******************************************************************************/

//
//#if !(EXTI15_10_IRQn)
//#define DECAIRQ_EXTI_IRQn       (40)
//#else
//#define DECAIRQ_EXTI_IRQn       (EXTI15_10_IRQn)
//#endif
//



#define DW1000_RSTn                 DW_RESET_Pin
#define DW1000_RSTn_GPIO            DW_RESET_GPIO_Port


//#define DECAIRQ                     DW_IRQn_Pin
//#define DECAIRQ_GPIO                DW_IRQn_GPIO_Port

#define TA_BOOT1                    GPIO_PIN_2
#define TA_BOOT1_GPIO               GPIOB

#define TA_RESP_DLY                 GPIO_PIN_0
#define TA_RESP_DLY_GPIO            GPIOC

#define TA_SW1_3                    GPIO_PIN_0
#define TA_SW1_4                    GPIO_PIN_1
#define TA_SW1_5                    GPIO_PIN_2
#define TA_SW1_6                    GPIO_PIN_3
#define TA_SW1_7                    GPIO_PIN_4
#define TA_SW1_8                    GPIO_PIN_5
#define TA_SW1_GPIO                 GPIOC

/****************************************************************************//**
 *
 *                              MACRO function
 *
 *******************************************************************************/

#define GPIO_ResetBits(x,y)             HAL_GPIO_WritePin(x,y, RESET)
#define GPIO_SetBits(x,y)               HAL_GPIO_WritePin(x,y, SET)
#define GPIO_ReadInputDataBit(x,y)      HAL_GPIO_ReadPin (x,y)


/* NSS pin is SW controllable */
//#define port_SPIx_set_chip_select()     HAL_GPIO_WritePin(DW_NSS_GPIO_Port, DW_NSS_Pin, GPIO_PIN_SET)
//#define port_SPIx_clear_chip_select()   HAL_GPIO_WritePin(DW_NSS_GPIO_Port, DW_NSS_Pin, GPIO_PIN_RESET)

/****************************************************************************//**
 *
 *                              port function prototypes
 *
 *******************************************************************************/

void Sleep(uint32_t Delay);
unsigned long portGetTickCnt(void);

#define S1_SWITCH_ON  (1)
#define S1_SWITCH_OFF (0)
//when switch (S1) is 'on' the pin is low
int port_is_boot1_on(uint16_t x);
int port_is_switch_on(uint16_t GPIOpin);
int port_is_boot1_low(void);

void port_wakeup_dw1000(UWBPortTypeDef *pports);
void port_wakeup_dw1000_fast(UWBPortTypeDef *pports);

void port_set_dw1000_slowrate(UWBPortTypeDef *pports);
void port_set_dw1000_fastrate(UWBPortTypeDef *pports);

void process_dwRSTn_irq(void);
void process_deca_irq(UWBPortTypeDef *);

void led_on(led_t led);
void led_off(led_t led);

int  peripherals_init(void);
void spi_peripheral_init(void);

void setup_DW1000RSTnIRQ(int enable);

void reset_DW1000(UWBPortTypeDef *pports);

ITStatus EXTI_GetITEnStatus(uint32_t x);

uint32_t port_GetEXT_IRQStatus(UWBPortTypeDef *pports);
uint32_t port_CheckEXT_IRQ(UWBPortTypeDef *pports);
void port_DisableEXT_IRQ(UWBPortTypeDef *pports);
void port_EnableEXT_IRQ(UWBPortTypeDef *pports);
extern uint32_t     HAL_GetTick(void);
HAL_StatusTypeDef   flush_report_buff(void);

#ifdef __cplusplus
}
#endif

#endif /* PORT_H_ */
/*
 * Taken from the Linux Kernel
 *
 */

#ifndef _LINUX_CIRC_BUF_H
#define _LINUX_CIRC_BUF_H 1

struct circ_buf {
    char *buf;
    int head;
    int tail;
};

/* Return count in buffer.  */
#define CIRC_CNT(head,tail,size) (((head) - (tail)) & ((size)-1))

/* Return space available, 0..size-1.  We always leave one free char
   as a completely full buffer has head == tail, which is the same as
   empty.  */
#define CIRC_SPACE(head,tail,size) CIRC_CNT((tail),((head)+1),(size))

/* Return count up to the end of the buffer.  Carefully avoid
   accessing head and tail more than once, so they can change
   underneath us without returning inconsistent results.  */
#define CIRC_CNT_TO_END(head,tail,size) \
    ({int end = (size) - (tail); \
      int n = ((head) + end) & ((size)-1); \
      n < end ? n : end;})

/* Return space available up to the end of the buffer.  */
#define CIRC_SPACE_TO_END(head,tail,size) \
    ({int end = (size) - 1 - (head); \
      int n = (end + (tail)) & ((size)-1); \
      n <= end ? n : end+1;})

#endif /* _LINUX_CIRC_BUF_H  */

