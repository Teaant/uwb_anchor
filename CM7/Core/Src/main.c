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
#include "rng.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "corecomm.h"
#include <math.h>
#include <uwb_mac.h>
#include "utilities.h"
#include "aoa.h"

#include "agent.h"
#include "corecomm.h"

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

#define RUN_PROTOCOL	1
#define TEST_TX_BUFFER	0

#define TEST_DEMO	0



#define TEST_ADD_NODE	0
#if(TEST_ADD_NODE)
volatile uint32_t test_id = 1;
volatile uint8_t interval = 1;
#endif
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

volatile int8_t enableCM4 = 0;

volatile AoADataTypeDef aoa_data[MAX_TAG] __attribute__ ((section(".shared")));
volatile PDoA_Struct_t pdoa_diags[3] __attribute__ ((section(".shared")));

volatile uint8_t ranging_num __attribute__ ((section(".shared")));

__attribute__((section(".shared"))) volatile uint8_t flag = 0;

__attribute__ ((section(".shared"))) volatile uint8_t rx_fail = 0;

__attribute__ ((section(".shared"))) volatile uint32_t error_status = 0;

__attribute__ ((section(".shared"))) volatile PDoA_Frame_t rxBuffer;

#define CONFIG_FLASH_ADDRESS	0x080E0000

#define CONFIG_FLASH_SECTOR		7

extern Flash_Config_t flash_config;

//volatile AoADiagnosticTypeDef aoa_diagnostic[DWT_NUM_DW_DEV] __attribute__ ((section(".shared")));

//volatile float aoa_calibration_table_raw[4][AOA_CALIBRATION_TABLE_LENGTH] __attribute__ ((section(".shared")));

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);

static uint32_t getRandom(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

extern UWB_Node_t uwb_node;


uint8_t is_fail = 0;


extern UART_HandleTypeDef huart2;

volatile uint8_t counts = 0;



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
	while ((__HAL_RCC_GET_FLAG(RCC_FLAG_D2CKRDY) != RESET) && (timeout-- > 0));
	if (timeout < 0) {
		Error_Handler();
		is_fail = 1;
	}
/* USER CODE END Boot_Mode_Sequence_1 */
  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  HAL_Delay(1000);
	//HAL_PWREx_ConfigSupply(PWR_DIRECT_SMPS_SUPPLY);
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
	timeout = 0xFFFFF;
	while ((__HAL_RCC_GET_FLAG(RCC_FLAG_D2CKRDY) == RESET) && (timeout-- > 0))
		;
	if (timeout < 0) {
		Error_Handler();
		is_fail = 1;
	}

/* USER CODE END Boot_Mode_Sequence_2 */

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_SPI6_Init();
  MX_USART2_UART_Init();
  MX_TIM15_Init();
  MX_TIM4_Init();
  MX_TIM6_Init();

  MX_RNG_Init();


  /* USER CODE BEGIN 2 */
	if (hrng.State == HAL_RNG_STATE_READY) {
		uwb_node.rand_ok = 1;
		//设置回调函数
		uwb_node.get_rand = getRandom;
	}

	if(Flash_ReadConfig(&flash_config)){
		if(flash_config.tx_power == 0xFFFFFFFF
				|| flash_config.tcp_server == 0xFFFFFFFF
				|| flash_config.tag_interval == 0xFFFFFFFF
				|| flash_config.comm_range == 0xFFFFFFFF){
			printf("first time to flash config, write default.\r\n");
			flash_config.tx_power = DEFAULT_TX_POWER;
			flash_config.tcp_server = 1;
			flash_config.tag_interval = INTERVAL;
			flash_config.comm_range = COMM_RANGE;
			if(Flash_WriteConfig(&flash_config) == HAL_OK){
				printf("write to flash success.\r\n");
			}else{
				printf("write to flash fail.\r\n");
			}
		}else{
			printf("read config.\r\n");
		}
	}

#if(USE_WIFI)
	//Configure WiFi
	HAL_GPIO_WritePin(WIFI_EN_GPIO_Port, WIFI_EN_Pin, GPIO_PIN_RESET);

	HAL_Delay(100);
	//使能ESP32
	HAL_GPIO_WritePin(WIFI_EN_GPIO_Port, WIFI_EN_Pin, GPIO_PIN_SET);

	HAL_Delay(500);
	Connect_Wifi();
#endif

#if(!TEST_DEMO)

	HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);

	initNode();

	for (int i = 0; i < DWT_NUM_DW_DEV; i++) {
		if (uwb_node.device->ports[i].avalible == 1) {
			/* Set expected response's delay and timeout. See NOTE 4, 5 and 6 below.
			 * As this example only handles one incoming frame with always the same delay and timeout, those values can be set here once for all. */
//#if(USE_LOG)
//			printf("DW1000 init successful.\r\n");
//#endif
			dwt_setrxaftertxdelay(0,  &uwb_node.device->ports[i]);
			dwt_setrxtimeout(0, &uwb_node.device->ports[i]);
			HAL_NVIC_ClearPendingIRQ(uwb_node.device->ports[i].exti_line);
			HAL_NVIC_EnableIRQ(uwb_node.device->ports[i].exti_line);
//			if(i == 0)
//			dwt_rxenable(DWT_START_RX_IMMEDIATE, &uwb_node.device->ports[i]);  //也并不需要
		}
	}



#endif

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
//	ENABLE_TIMER2();  // 1000000
//	//32 bit 是 4294967295
	start_run();

	U_Task u_task = NULL;
	uint16_t param = 0;

	while (1) {
		u_task = dequeueTask(&param);
		if (u_task) {
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48|RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 50;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
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
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SPI6|RCC_PERIPHCLK_SPI1;
  PeriphClkInitStruct.PLL2.PLL2M = 4;
  PeriphClkInitStruct.PLL2.PLL2N = 10;
  PeriphClkInitStruct.PLL2.PLL2P = 2;
  PeriphClkInitStruct.PLL2.PLL2Q = 2;
  PeriphClkInitStruct.PLL2.PLL2R = 2;
  PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_3;
  PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOMEDIUM;
  PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
  PeriphClkInitStruct.Spi123ClockSelection = RCC_SPI123CLKSOURCE_PLL2;
  PeriphClkInitStruct.Spi6ClockSelection = RCC_SPI6CLKSOURCE_PLL2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
static uint32_t getRandom(void){
	uint32_t num = 0;
	HAL_RNG_GenerateRandomNumber(&hrng,  &num);
	return num;
}


HAL_StatusTypeDef Flash_WriteConfig(Flash_Config_t *cfg)
{
    HAL_StatusTypeDef status;
    uint32_t address = CONFIG_FLASH_ADDRESS;
//    uint32_t *data = (uint32_t *)cfg;

    HAL_FLASH_Unlock();

    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t SectorError;

    EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
    EraseInitStruct.Banks = FLASH_BANK_1;
    EraseInitStruct.Sector = CONFIG_FLASH_SECTOR;
    EraseInitStruct.NbSectors = 1;
    EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3; // 2.7V~3.6V

    status = HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError);
    if (status != HAL_OK)
    {
        HAL_FLASH_Lock();
        return status;
    }

    //该系列MCU一个Flash字是8 个32-bit大小，刚好我设置的是cfg这里
    status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, address, (uint32_t)cfg);
    if (status != HAL_OK)
    {
        HAL_FLASH_Lock();
        return status;
    }

    HAL_FLASH_Lock();

    return HAL_OK;
}

int Flash_ReadConfig(Flash_Config_t *cfg)
{
	Flash_Config_t *ptr = (Flash_Config_t *)CONFIG_FLASH_ADDRESS;
    *cfg = *ptr;
    return 1;
}

void Software_Reset(void)
{
    HAL_NVIC_SystemReset();
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
  //Resp消息相关定时器
	if (htim->Instance == TIM6) {
		DISABLE_TIMER6();
#if(MY_ROLE == TAG)
		uwb_node.ptag_struct->timer6_callback();
#else
		timer6_callback();
#endif   // MY_ROLE

	}

	if(htim->Instance == TIM15){
		DISABLE_TIMER15();

#if(MY_ROLE == TAG)
		//wake up the tag ~
		uwb_node.ptag_struct->timer15_callback();
#else    // ANCHOR
		//随机数获取成功了
//#if(USE_LOG)
//		uint32_t random = uwb_node.get_rand();
//		printf("random = %lu. \r\n", random);
//#endif
		timer15_callback();

#endif
	}


	if(htim->Instance == TIM4){
		//anchor_absence
		DISABLE_TIMER4();
#if(MY_ROLE == TAG)
		Tag_lose_anchor();
#else
		enqueueTask(Log_Data, 0);

#endif
	}


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
