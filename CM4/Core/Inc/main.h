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
#define UWB1A_IRQ_Pin GPIO_PIN_0
#define UWB1A_IRQ_GPIO_Port GPIOA
#define UWB1B_IRQ_Pin GPIO_PIN_1
#define UWB1B_IRQ_GPIO_Port GPIOA
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
#define SYNC_ENABLE_Pin GPIO_PIN_12
#define SYNC_ENABLE_GPIO_Port GPIOE
#define LED2_Pin GPIO_PIN_14
#define LED2_GPIO_Port GPIOD

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
