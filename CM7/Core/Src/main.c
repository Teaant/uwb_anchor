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
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "uwb_mac.h"

#include "task_manager.h"

#include "corecomm.h"
#include <math.h>
#include "utilities.h"
#include "aoa.h"

#if(RANGING_ROLE == ANCHOR)
#include "uwb_mac_anchor.h"
#else
#include "uwb_mac_tag.h"
#endif


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


volatile int8_t enableCM4 = 0;


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

extern UWB_Node_t uwb_node;
extern Timer_t my_timer;

//
//extern TIM_HandleTypeDef htim6;
//extern TIM_HandleTypeDef htim7;


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */
/* USER CODE BEGIN Boot_Mode_Sequence_0 */
	int32_t timeout;
/* USER CODE END Boot_Mode_Sequence_0 */

/* USER CODE BEGIN Boot_Mode_Sequence_1 */
	/* Wait until CPU2 boots and enters in stop mode or timeout*/
	timeout = 0xFFFF;
	while ((__HAL_RCC_GET_FLAG(RCC_FLAG_D2CKRDY) != RESET) && (timeout-- > 0))
		;
	if (timeout < 0) {
//		Error_Handler();   //CM4没有准备好，那便算了
	}
/* USER CODE END Boot_Mode_Sequence_1 */
  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

/* Configure the peripherals common clocks */
  PeriphCommonClock_Config();
/* USER CODE BEGIN Boot_Mode_Sequence_2 */
	/* When system initialization is finished, Cortex-M7 will release Cortex-M4 by means of
	 HSEM notification */
	/*HW semaphore Clock enable*/
	__HAL_RCC_HSEM_CLK_ENABLE();
	/*Take HSEM */
	HAL_HSEM_FastTake(HSEM_ID_0);
	/*Release HSEM in order to notify the CPU2(CM4)*/
	HAL_HSEM_Release(HSEM_ID_0, 0);
	/* wait until CPU2 wakes up from stop mode */
	timeout = 0xFFFF;
	while ((__HAL_RCC_GET_FLAG(RCC_FLAG_D2CKRDY) == RESET) && (timeout-- > 0))
		;
	if (timeout < 0) {
//		Error_Handler();
	}
/* USER CODE END Boot_Mode_Sequence_2 */

  /* USER CODE BEGIN SysInit */

//	HAL_PWREx_ConfigSupply(PWR_DIRECT_SMPS_SUPPLY);

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_SPI1_Init();
  MX_SPI2_Init();
  MX_SPI3_Init();
  MX_SPI4_Init();
  MX_SPI6_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  HAL_NVIC_DisableIRQ(EXTI0_IRQn);
  HAL_NVIC_DisableIRQ(EXTI1_IRQn);
  HAL_NVIC_DisableIRQ(EXTI3_IRQn);
  HAL_NVIC_DisableIRQ(EXTI4_IRQn);
  HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
  HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);

  /**
   * Tanya_edit: no need right now
   */
//   bufferInit();

   initNode(RANGING_ROLE, &htim2);

   HAL_TIM_Base_Start(&htim3);

#if(RANGING_ROLE == TAG)

   HAL_TIM_OC_Start(&htim2, TIM_CHANNEL_2);
   HAL_TIM_OC_Start(&htim2, TIM_CHANNEL_3);
   HAL_TIM_OC_Start(&htim2, TIM_CHANNEL_4);

#else
   HAL_TIM_OC_Start(&htim2, TIM_CHANNEL_1);
   HAL_TIM_OC_Start(&htim2, TIM_CHANNEL_2);
   HAL_TIM_OC_Start(&htim2, TIM_CHANNEL_3);
   HAL_TIM_OC_Start(&htim2, TIM_CHANNEL_4);

#endif

 	for(int i=0;i<DWT_NUM_DW_DEV;i++)
 	{
 		if(uwb_node.device->ports[i].avalible == 1)
 		{
 		    /* Set expected response's delay and timeout. See NOTE 4, 5 and 6 below.
 		     * As this example only handles one incoming frame with always the same delay and timeout, those values can be set here once for all. */
// 			dwt_setrxaftertxdelay(0, pports);  //若是不设置呢，我希望在代码中设置的，若是不进入rx我希�??
 			dwt_setrxtimeout(0, &uwb_node.device->ports[i]);
 			HAL_NVIC_ClearPendingIRQ(uwb_node.device->ports[i].exti_line);
 			HAL_NVIC_EnableIRQ(uwb_node.device->ports[i].exti_line);
 		}
 	}

// 	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, GPIO_PIN_SET);  //This is to do what�?  to synchronize the PDoA board

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
#if(RANGING_ROLE == ANCHOR)
 	prepare_beacon(uwb_node.id);
#endif

 	U_Task u_task = NULL;
 	uint16_t param = 0;

	while (1) {

		u_task = dequeueTask(&param);
		if(u_task){
			u_task(param);
		}
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_DIRECT_SMPS_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  __HAL_RCC_SYSCFG_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 60;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SPI6|RCC_PERIPHCLK_SPI3
                              |RCC_PERIPHCLK_SPI2|RCC_PERIPHCLK_SPI1
                              |RCC_PERIPHCLK_SPI4;
  PeriphClkInitStruct.PLL2.PLL2M = 4;
  PeriphClkInitStruct.PLL2.PLL2N = 32;
  PeriphClkInitStruct.PLL2.PLL2P = 8;
  PeriphClkInitStruct.PLL2.PLL2Q = 8;
  PeriphClkInitStruct.PLL2.PLL2R = 2;
  PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_3;
  PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOWIDE;
  PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
  PeriphClkInitStruct.Spi123ClockSelection = RCC_SPI123CLKSOURCE_PLL2;
  PeriphClkInitStruct.Spi45ClockSelection = RCC_SPI45CLKSOURCE_PLL2;
  PeriphClkInitStruct.Spi6ClockSelection = RCC_SPI6CLKSOURCE_PLL2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
/**
 *  TIMx->CCR2 = OC_Config->Pulse;   rw/r 是什么意思？
 */
//要�?�虑各种的这边的状�?�之类的嘛？
void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim){

#if(RANGING_ROLE == TAG)
	if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2){
		if(my_timer.callback){
			my_timer.callback();
		}
	}
	if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_3) {
#if(Tanya_Test_Timer)
		HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin);
		Reset_Timer();   //�?新
		Tag_Set_Compare(40, test_callback);
#else
		//休眠
		DISABLE_COMP3(htim);
		HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin);
		tag_wakeup_radio();
#endif

	}
	if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_4) {

		DISABLE_COMP4(htim);
		HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
		/**
		 * @TODO  长时间未收到锚节点的beacon
		 */
		Tag_lose_anchor();
	}
#else

	if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1){
		if(my_timer.callback){
			HAL_GPIO_TogglePin(LED3_GPIO_Port, LED1_Pin);
			my_timer.callback();
		}
	}
	if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2){
			if(my_timer.callback1){
				my_timer.callback1(my_timer.param1);
			}
		}
	if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_3){
			if(my_timer.callback2){
				my_timer.callback2(my_timer.param2);
			}
		}
	if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_4){
			if(my_timer.callback3){
				my_timer.callback3(my_timer.param3);
			}
		}

#endif

}
/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM16 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM16) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */
//  if (htim->Instance == TIM2) {
////	  Inc_Uwb_Tick();
//	  //�??????????要restart?
//
//	  //zaiguancha
//   }
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
