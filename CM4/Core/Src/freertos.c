/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "agent.h"
#include "modbus.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
extern UART_HandleTypeDef huart2;


extern holding_reg_params_t hold_data;
extern input_reg_params_t input_data;

uint8_t uart_rcv_buf[256];
uint8_t uart_msg[256];



/* USER CODE END Variables */
osThreadId LedblinkTaskHandle;
osThreadId MsgParseTaskHandle;
osSemaphoreId MsgRcvSemHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void LedBlink(void const * argument);
void StartTask02(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* definition and creation of MsgRcvSem */
  osSemaphoreDef(MsgRcvSem);
  MsgRcvSemHandle = osSemaphoreCreate(osSemaphore(MsgRcvSem), 1);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of LedblinkTask */
  osThreadDef(LedblinkTask, LedBlink, osPriorityBelowNormal, 0, 128);
  LedblinkTaskHandle = osThreadCreate(osThread(LedblinkTask), NULL);

  /* definition and creation of MsgParseTask */
  osThreadDef(MsgParseTask, StartTask02, osPriorityNormal, 0, 128);
  MsgParseTaskHandle = osThreadCreate(osThread(MsgParseTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_LedBlink */
/**
  * @brief  Function implementing the LedblinkTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_LedBlink */
void LedBlink(void const * argument)
{
  /* USER CODE BEGIN LedBlink */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END LedBlink */
}

/* USER CODE BEGIN Header_StartTask02 */
/**
* @brief Function implementing the MsgParseTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask02 */
void StartTask02(void const * argument)
{
  /* USER CODE BEGIN StartTask02 */
  /* Infinite loop */
	osStatus ret = osOK;
	HAL_UART_Receive_DMA_ToIdle(&huart2, uart_rcv_buf, 256);
  for(;;)
  {
	  //osWaitForever
	 ret = osSemaphoreWait(MsgRcvSemHandle, osWaitForever);
	 if(ret == osOK){
	  //process msg
		 HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);  //确认是接收到了 ~
		 parse_modbus_msg(uart_msg);
	  //printf((char*)uart_msg);
	 }
  }
  /* USER CODE END StartTask02 */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size){

	//HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
	memcpy(uart_msg, uart_rcv_buf, Size);
	osSemaphoreRelease(MsgRcvSemHandle);
	HAL_UART_Receive_DMA_ToIdle(huart, uart_rcv_buf, 256);   //�?�缓冲队列？

}

/* USER CODE END Application */
