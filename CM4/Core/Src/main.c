/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "spi.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "gpio.h"
#include <math.h>

#include "aoa.h"

#include "agent.h"
#include "corecomm.h"

#include "uwb.h"

#include "uwb_msg.h"

#include "task_manager.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#ifndef HSEM_ID_0
#define HSEM_ID_0 (0U) /* HW semaphore 0*/
#endif

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
volatile AoADataTypeDef aoa_data[MAX_TAG] __attribute__ ((section(".shared")));
volatile PDoA_Struct_t pdoa_diags[3] __attribute__ ((section(".shared")));

volatile uint8_t ranging_num __attribute__ ((section(".shared")));

__attribute__((section(".shared"))) volatile uint8_t flag = 0;

__attribute__ ((section(".shared"))) volatile uint8_t rx_fail = 0;

__attribute__ ((section(".shared"))) volatile uint32_t error_status = 0;

__attribute__ ((section(".shared"))) volatile PDoA_Frame_t rxBuffer;

extern UWBDef UWB;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

/* USER CODE BEGIN Boot_Mode_Sequence_1 */
	/*HW semaphore Clock enable*/
	__HAL_RCC_HSEM_CLK_ENABLE();
//	/* Activate HSEM notification for Cortex-M4*/
	HAL_HSEM_ActivateNotification(__HAL_HSEM_SEMID_TO_MASK(HSEM_ID_0));
	/*
	 Domain D2 goes to STOP mode (Cortex-M4 in deep-sleep) waiting for Cortex-M7 to
	 perform system initialization (system clock config, external memory configuration.. )
	 */
	HAL_PWREx_ClearPendingEvent();
	HAL_PWREx_EnterSTOPMode(PWR_MAINREGULATOR_ON, PWR_STOPENTRY_WFE,
			PWR_D2_DOMAIN);
	/* Clear HSEM flag */
	__HAL_HSEM_CLEAR_FLAG(__HAL_HSEM_SEMID_TO_MASK(HSEM_ID_0));

/* USER CODE END Boot_Mode_Sequence_1 */
  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  HAL_Delay(5000);
  /* USER CODE END Init */

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_DMA_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */

  MX_GPIO_Init();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	HAL_NVIC_DisableIRQ(EXTI0_IRQn);
	HAL_NVIC_DisableIRQ(EXTI1_IRQn);

	uwbInit(0);

	for (int i = 1; i < DWT_NUM_DW_DEV; i++) {
		if (UWB.ports[i].avalible == 1) {
			dwt_setrxaftertxdelay(0, &UWB.ports[i]);
			dwt_setrxtimeout(0, &UWB.ports[i]);
			HAL_NVIC_ClearPendingIRQ(UWB.ports[i].exti_line);
			HAL_NVIC_EnableIRQ(UWB.ports[i].exti_line);
		}
	}

  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, GPIO_PIN_SET); //This is to do what�??  to synchronize the PDoA board

  U_Task u_task = NULL;
  uint16_t para = 0;
	while (1) {
		u_task = dequeueTask(&para);
		if(u_task){
			u_task(para);
		}

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
//	  HAL_UART_Transmit(&huart2, (uint8_t*) "AT+RST\r\n", 10, 200);
	}
  /* USER CODE END 3 */
}

/* USER CODE BEGIN 4 */
void HAL_HSEM_FreeCallback(uint32_t statusreg){

	if(__HAL_HSEM_SEMID_TO_MASK(Enable_PDoA) & statusreg){
		enable_pdoa();
		HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
	}
	if(__HAL_HSEM_SEMID_TO_MASK(Disable_PDoA) & statusreg){
		disable_pdoa();
	}

	if (__HAL_HSEM_SEMID_TO_MASK(Process_PDoA) & statusreg) {
		disable_pdoa();
		enqueueTask(process_pdoa, 0);
	}

}
/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM17 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM17) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */
	//按照所说的，还是不要在中断当中进行这边的计算了好吧 ~
//	if (htim->Instance == TIM7) {
//		/**
//		 * 计算的power最大在-170左右，相邻的相差大约20 ~
//		 */
//		HAL_TIM_Base_Stop_IT(&htim7);
//		enqueueTask(process_pdoa, 0);
//
//	}
  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
