/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.c
  * @brief   This file provides code for the configuration
  *          of the USART instances.
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
#include "usart.h"

/* USER CODE BEGIN 0 */
#define WiFiMaxRcvSize 100
uint8_t WiFiRcvBuffer[100];

/* USER CODE END 0 */

UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart2_rx;

/* USART2 init function */

void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 2304000;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart2, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart2, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  if(uartHandle->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspInit 0 */

  /* USER CODE END USART2_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART2;
    PeriphClkInitStruct.PLL3.PLL3M = 4;
    PeriphClkInitStruct.PLL3.PLL3N = 16;
    PeriphClkInitStruct.PLL3.PLL3P = 2;
    PeriphClkInitStruct.PLL3.PLL3Q = 4;
    PeriphClkInitStruct.PLL3.PLL3R = 4;
    PeriphClkInitStruct.PLL3.PLL3RGE = RCC_PLL3VCIRANGE_3;
    PeriphClkInitStruct.PLL3.PLL3VCOSEL = RCC_PLL3VCOWIDE;
    PeriphClkInitStruct.PLL3.PLL3FRACN = 0;
    PeriphClkInitStruct.Usart234578ClockSelection = RCC_USART234578CLKSOURCE_PLL3;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    /* USART2 clock enable */
    __HAL_RCC_USART2_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART2 GPIO Configuration
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART2 DMA Init */
    /* USART2_RX Init */
    hdma_usart2_rx.Instance = DMA1_Stream0;
    hdma_usart2_rx.Init.Request = DMA_REQUEST_USART2_RX;
    hdma_usart2_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart2_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart2_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart2_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart2_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart2_rx.Init.Mode = DMA_NORMAL;
    hdma_usart2_rx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_usart2_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_usart2_rx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle,hdmarx,hdma_usart2_rx);

  /* USER CODE BEGIN USART2_MspInit 1 */

  /* USER CODE END USART2_MspInit 1 */
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

  if(uartHandle->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspDeInit 0 */

  /* USER CODE END USART2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART2_CLK_DISABLE();

    /**USART2 GPIO Configuration
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2|GPIO_PIN_3);

    /* USART2 DMA DeInit */
    HAL_DMA_DeInit(uartHandle->hdmarx);
  /* USER CODE BEGIN USART2_MspDeInit 1 */

  /* USER CODE END USART2_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
int __io_putchar(int ch){
	HAL_UART_Transmit(&huart2, (uint8_t*)ch, 1, 10);
	return ch;
}


void connect_wifi(void) {

	huart2.Instance->CR1 &= ~(0x0001 << 4);
	//初始化DMA
	HAL_DMA_Init(&hdma_usart2_rx);
	//�????????????????????????????????????????????启DMA接收
	HAL_UART_Receive_DMA(&huart2, WiFiRcvBuffer, WiFiMaxRcvSize);
	//	//复位
	HAL_GPIO_WritePin(WIFI_EN_GPIO_Port, WIFI_EN_Pin, GPIO_PIN_RESET);
	//
	HAL_Delay(1000);
	HAL_GPIO_WritePin(WIFI_EN_GPIO_Port, WIFI_EN_Pin, GPIO_PIN_SET);
	HAL_Delay(1000);

	HAL_UART_Transmit(&huart2, (uint8_t*) "+++", 3, 200);
	HAL_Delay(1000);
	memset(WiFiRcvBuffer, '\0', WiFiMaxRcvSize);
	//初始化DMA
	HAL_DMA_Init(&hdma_usart2_rx);
	//�????????????????????????????????????????????启DMA接收
	HAL_UART_Receive_DMA(&huart2, WiFiRcvBuffer, WiFiMaxRcvSize);

	printf("AT+CIPMODE=0\r\n");
	HAL_Delay(1000);
	memset(WiFiRcvBuffer, '\0', WiFiMaxRcvSize);
	//初始化DMA
	HAL_DMA_Init(&hdma_usart2_rx);
	//�????????????????????????????????????????????启DMA接收
	HAL_UART_Receive_DMA(&huart2, WiFiRcvBuffer, WiFiMaxRcvSize);

	printf("AT+CIPCLOSE\r\n");
	HAL_Delay(1000);
	memset(WiFiRcvBuffer, '\0', WiFiMaxRcvSize);
	//初始化DMA
	HAL_DMA_Init(&hdma_usart2_rx);
	//�????????????????????????????????????????????启DMA接收
	HAL_UART_Receive_DMA(&huart2, WiFiRcvBuffer, WiFiMaxRcvSize);

	printf("AT+RST\r\n");
	HAL_Delay(2000);
	memset(WiFiRcvBuffer, '\0', WiFiMaxRcvSize);
	//初始化DMA
	HAL_DMA_Init(&hdma_usart2_rx);
	//�????????????????????????????????????????????启DMA接收
	HAL_UART_Receive_DMA(&huart2, WiFiRcvBuffer, WiFiMaxRcvSize);

	do {
		//使用时将UART2波特率设置为2304000
		//假设ESP32波特率也 ?2304000
		//则直接进行AT指令
		HAL_DMA_Init(&hdma_usart2_rx);
		HAL_UART_Receive_DMA(&huart2, WiFiRcvBuffer, WiFiMaxRcvSize);
		memset(WiFiRcvBuffer, '\0', WiFiMaxRcvSize);
		printf("ATE0\r\n");
		HAL_Delay(500);
		if (WiFiRcvBuffer[2] == 'O' && WiFiRcvBuffer[3] == 'K') {
			//若指令正确返回，说明波特率匹配，直接 ?出即 ?
			break;
		}
		HAL_DMA_Init(&hdma_usart2_rx);
		HAL_UART_Receive_DMA(&huart2, WiFiRcvBuffer, WiFiMaxRcvSize);
		//若指令没有正确返回，则将UART2波特率设置为115200，再次进行AT指令
		huart2.Init.BaudRate = 115200;
		HAL_UART_Init(&huart2);
		memset(WiFiRcvBuffer, '\0', WiFiMaxRcvSize);
		printf("ATE0\r\n");
		HAL_Delay(500);
		if (WiFiRcvBuffer[2] == 'O' && WiFiRcvBuffer[3] == 'K') {
			//若指令正确返回，说明波特率匹配，将其设置 ?2304000
			break;

		}
		huart2.Init.BaudRate = 2304000;
		HAL_UART_Init(&huart2);

	} while (WiFiRcvBuffer[2] != 'O' || WiFiRcvBuffer[3] != 'K');

	do {
		//初始化DMA
		HAL_DMA_Init(&hdma_usart2_rx);
		//DMA接收
		HAL_UART_Receive_DMA(&huart2, WiFiRcvBuffer, WiFiMaxRcvSize);
		memset(WiFiRcvBuffer, '\0', WiFiMaxRcvSize);
		//SoftAP+Station
		printf("AT+CWMODE=3\r\n");
		HAL_Delay(500);
		//		printf("%s",WiFiRcvBuffer);


	} while (WiFiRcvBuffer[2] != 'O' || WiFiRcvBuffer[3] != 'K');

	uint8_t flag = 0;
	do {

		uint8_t tempstr[] = "OK\r\n\0\0";

		memset(WiFiRcvBuffer, '\0', WiFiMaxRcvSize);
		//初始化DMA
		HAL_DMA_Init(&hdma_usart2_rx);
		//DMA接收
		HAL_UART_Receive_DMA(&huart2, WiFiRcvBuffer, WiFiMaxRcvSize);
		//连接WiFi
		//		printf("AT+CWJAP=\"AU3#506\",\"password\"\r\n");
		printf("AT+CWJAP=\"Tanya\",\"tanyanderedian\"\r\n");
		//		printf("AT+CWJAP=\"zhuran\",\"10121012\"\r\n");
		HAL_Delay(1000);
		//		printf("%s",WiFiRcvBuffer);

		for (int i = 0; i <= 80; i++) {
			if (memcmp(tempstr, &WiFiRcvBuffer[i], (sizeof(tempstr) - 1))
					== 0) {
				flag = 1;
				break;
			}
		}
		//	}while((WiFiRcvBuffer[5] != 'C' || WiFiRcvBuffer[6] != 'O') && (WiFiRcvBuffer[22] != 'C' || WiFiRcvBuffer[23] != 'O'));
	} while (flag != 1);

	HAL_DMA_Init(&hdma_usart2_rx);

	do {
		//		printf("AT+CIPCLOSE");

		memset(WiFiRcvBuffer, '\0', WiFiMaxRcvSize);

		//连接TCP服务�????????????????????????????????????????????
		//		printf("AT+CIPSTART=\"UDP\",\"192.168.1.104\",8080\r\n");

		//初始化DMA
		HAL_DMA_Init(&hdma_usart2_rx);
		//DMA接收
		HAL_UART_Receive_DMA(&huart2, WiFiRcvBuffer, WiFiMaxRcvSize);
		//		UDP
		//		printf("AT+CIPSTART=\"UDP\",\"192.168.31.135\",8080\r\n");
		printf("AT+CIPSTART=\"UDP\",\"192.168.137.1\",8080\r\n");
		//		printf("AT+CIPSTART=\"UDP\",\"192.168.1.127\",8080\r\n");
		//		printf("AT+CIPSTART=\"UDP\",\"192.168.1.104\",8090\r\n");

		//		printf("AT+CIPSTART=\"UDP\",\"192.168.43.1\",8080\r\n");

		//printf("AT+CIPSTART=\"TCP\",\"192.168.137.1\",8080\r\n");

		//		printf("%s",WiFiRcvBuffer);
		HAL_Delay(1000);



	} while (WiFiRcvBuffer[0] != 'C');
	do {

		memset(WiFiRcvBuffer, '\0', WiFiMaxRcvSize);
		//初始化DMA
		HAL_DMA_Init(&hdma_usart2_rx);
		//DMA接收
		HAL_UART_Receive_DMA(&huart2, WiFiRcvBuffer, WiFiMaxRcvSize);
		//获取IP
		printf("AT+CIFSR\r\n");
		HAL_Delay(1000);
		//		printf("%s",WiFiRcvBuffer);
	} while (WiFiRcvBuffer[1] != 'C' || WiFiRcvBuffer[3] != 'F');
	//wifi连接成功后使用ID地址作为ID
	//	agent.ID = 100*(WiFiRcvBuffer[85]-48)+10*(WiFiRcvBuffer[86]-48)+(WiFiRcvBuffer[87]-48);

	do {
		//初始化DMA
		HAL_DMA_Init(&hdma_usart2_rx);
		//DMA接收
		HAL_UART_Receive_DMA(&huart2, WiFiRcvBuffer, WiFiMaxRcvSize);
		memset(WiFiRcvBuffer, '\0', WiFiMaxRcvSize);

		//使能透传模式浣胯兘閫忎紶妯�?�紡
		printf("AT+CIPMODE=1\r\n");
		HAL_Delay(1000);
		//		printf("%s",WiFiRcvBuffer);

	} while (WiFiRcvBuffer[2] != 'O' || WiFiRcvBuffer[3] != 'K');

	do {

		memset(WiFiRcvBuffer, '\0', WiFiMaxRcvSize);
		//初始化DMA
		HAL_DMA_Init(&hdma_usart2_rx);
		//DMA接收
		HAL_UART_Receive_DMA(&huart2, WiFiRcvBuffer, WiFiMaxRcvSize);
		//发�?�模�????????????????????????????????????????????
		printf("AT+CIPSEND\r\n");
		HAL_Delay(1000);

	} while (WiFiRcvBuffer[2] != 'O' || WiFiRcvBuffer[3] != 'K');

	printf("ESP32 connected.\r\n");
}

/* USER CODE END 1 */
