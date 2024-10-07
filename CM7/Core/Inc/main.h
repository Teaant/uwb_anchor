/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define UWB4_SPICLK_Pin GPIO_PIN_2
#define UWB4_SPICLK_GPIO_Port GPIOE
#define UWB4A_IRQ_Pin GPIO_PIN_3
#define UWB4A_IRQ_GPIO_Port GPIOE
#define UWB4A_IRQ_EXTI_IRQn EXTI3_IRQn
#define UWB4B_IRQ_Pin GPIO_PIN_4
#define UWB4B_IRQ_GPIO_Port GPIOE
#define UWB4B_IRQ_EXTI_IRQn EXTI4_IRQn
#define UWB4_SPIMISO_Pin GPIO_PIN_5
#define UWB4_SPIMISO_GPIO_Port GPIOE
#define UWB4_SPIMOSI_Pin GPIO_PIN_6
#define UWB4_SPIMOSI_GPIO_Port GPIOE
#define UWB4_WAKEUP_Pin GPIO_PIN_13
#define UWB4_WAKEUP_GPIO_Port GPIOC
#define UWB4A_RSTn_Pin GPIO_PIN_0
#define UWB4A_RSTn_GPIO_Port GPIOF
#define UWB4B_RSTn_Pin GPIO_PIN_1
#define UWB4B_RSTn_GPIO_Port GPIOF
#define UWB4A_SPICSn_Pin GPIO_PIN_2
#define UWB4A_SPICSn_GPIO_Port GPIOF
#define UWB4B_SPICSn_Pin GPIO_PIN_3
#define UWB4B_SPICSn_GPIO_Port GPIOF
#define UWB1A_IRQ_Pin GPIO_PIN_0
#define UWB1A_IRQ_GPIO_Port GPIOA
#define UWB1A_IRQ_EXTI_IRQn EXTI0_IRQn
#define UWB1B_IRQ_Pin GPIO_PIN_1
#define UWB1B_IRQ_GPIO_Port GPIOA
#define UWB1B_IRQ_EXTI_IRQn EXTI1_IRQn
#define UWB1A_SPICSn_Pin GPIO_PIN_4
#define UWB1A_SPICSn_GPIO_Port GPIOA
#define UWB1_SPICLK_Pin GPIO_PIN_5
#define UWB1_SPICLK_GPIO_Port GPIOA
#define UWB1_SPIMISO_Pin GPIO_PIN_6
#define UWB1_SPIMISO_GPIO_Port GPIOA
#define UWB1_SPIMOSI_Pin GPIO_PIN_7
#define UWB1_SPIMOSI_GPIO_Port GPIOA
#define UWB1_WAKEUP_Pin GPIO_PIN_5
#define UWB1_WAKEUP_GPIO_Port GPIOC
#define UWB1B_SPICSn_Pin GPIO_PIN_0
#define UWB1B_SPICSn_GPIO_Port GPIOB
#define UWB1A_RSTn_Pin GPIO_PIN_1
#define UWB1A_RSTn_GPIO_Port GPIOB
#define UWB1B_RSTn_Pin GPIO_PIN_2
#define UWB1B_RSTn_GPIO_Port GPIOB
#define UWB2A_SPICSn_Pin GPIO_PIN_14
#define UWB2A_SPICSn_GPIO_Port GPIOF
#define UWB2B_SPICSn_Pin GPIO_PIN_15
#define UWB2B_SPICSn_GPIO_Port GPIOF
#define SYNC_ENABLE_Pin GPIO_PIN_12
#define SYNC_ENABLE_GPIO_Port GPIOE
#define UWB2_WAKEUP_Pin GPIO_PIN_12
#define UWB2_WAKEUP_GPIO_Port GPIOB
#define UWB2_SPICLK_Pin GPIO_PIN_13
#define UWB2_SPICLK_GPIO_Port GPIOB
#define UWB2_SPIMISO_Pin GPIO_PIN_14
#define UWB2_SPIMISO_GPIO_Port GPIOB
#define UWB2_SPIMOSI_Pin GPIO_PIN_15
#define UWB2_SPIMOSI_GPIO_Port GPIOB
#define UWB2A_IRQ_Pin GPIO_PIN_8
#define UWB2A_IRQ_GPIO_Port GPIOD
#define UWB2A_IRQ_EXTI_IRQn EXTI9_5_IRQn
#define UWB2B_IRQ_Pin GPIO_PIN_9
#define UWB2B_IRQ_GPIO_Port GPIOD
#define UWB2B_IRQ_EXTI_IRQn EXTI9_5_IRQn
#define UWB2A_RSTn_Pin GPIO_PIN_10
#define UWB2A_RSTn_GPIO_Port GPIOD
#define UWB2B_RSTn_Pin GPIO_PIN_11
#define UWB2B_RSTn_GPIO_Port GPIOD
#define LED1_Pin GPIO_PIN_13
#define LED1_GPIO_Port GPIOD
#define LED2_Pin GPIO_PIN_14
#define LED2_GPIO_Port GPIOD
#define LED3_Pin GPIO_PIN_15
#define LED3_GPIO_Port GPIOD
#define UWB3A_RSTn_Pin GPIO_PIN_8
#define UWB3A_RSTn_GPIO_Port GPIOA
#define UWB3B_RSTn_Pin GPIO_PIN_9
#define UWB3B_RSTn_GPIO_Port GPIOA
#define UWB3A_IRQ_Pin GPIO_PIN_10
#define UWB3A_IRQ_GPIO_Port GPIOA
#define UWB3A_IRQ_EXTI_IRQn EXTI15_10_IRQn
#define UWB3B_IRQ_Pin GPIO_PIN_15
#define UWB3B_IRQ_GPIO_Port GPIOA
#define UWB3B_IRQ_EXTI_IRQn EXTI15_10_IRQn
#define UWB3_SPICLK_Pin GPIO_PIN_10
#define UWB3_SPICLK_GPIO_Port GPIOC
#define UWB3_SPIMISO_Pin GPIO_PIN_11
#define UWB3_SPIMISO_GPIO_Port GPIOC
#define UWB3_SPIMOSI_Pin GPIO_PIN_12
#define UWB3_SPIMOSI_GPIO_Port GPIOC
#define UWB3_WAKEUP_Pin GPIO_PIN_0
#define UWB3_WAKEUP_GPIO_Port GPIOD
#define UWB3A_SPICSn_Pin GPIO_PIN_1
#define UWB3A_SPICSn_GPIO_Port GPIOD
#define UWB3B_SPICSn_Pin GPIO_PIN_2
#define UWB3B_SPICSn_GPIO_Port GPIOD
#define UWB_RSTn_Pin GPIO_PIN_10
#define UWB_RSTn_GPIO_Port GPIOG
#define UWB_IRQn_Pin GPIO_PIN_11
#define UWB_IRQn_GPIO_Port GPIOG
#define UWB_IRQn_EXTI_IRQn EXTI15_10_IRQn
#define UWB_MISO_Pin GPIO_PIN_12
#define UWB_MISO_GPIO_Port GPIOG
#define UWB_SCK_Pin GPIO_PIN_13
#define UWB_SCK_GPIO_Port GPIOG
#define UWB_MOSI_Pin GPIO_PIN_14
#define UWB_MOSI_GPIO_Port GPIOG
#define UWB_SPICSn_Pin GPIO_PIN_15
#define UWB_SPICSn_GPIO_Port GPIOG

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
