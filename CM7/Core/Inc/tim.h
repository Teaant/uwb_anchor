/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    tim.h
  * @brief   This file contains all the function prototypes for
  *          the tim.c file
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
#ifndef __TIM_H__
#define __TIM_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern TIM_HandleTypeDef htim4;

extern TIM_HandleTypeDef htim6;

extern TIM_HandleTypeDef htim15;

/* USER CODE BEGIN Private defines */
//1MHz
#define TIMER6_4MS				3999
#define TIMER6_12MS				11999

#define TIMER6_7MS	6999
#define TIMER6_8MS	7999
#define TIMER6_9MS	8999	//BOP 1,3
#define TIMER6_10MS	9999
#define TIMER6_11MS	10999
#define TIMER6_18MS	17999

#define ENABLE_TIMER6()			__HAL_TIM_CLEAR_FLAG(&htim6, TIM_FLAG_UPDATE);\
								__HAL_TIM_ENABLE(&htim6)

//TIMx->ARR = (uint32_t)Structure->Period;
#define ENABLE_TIMER6_ARR(period)	__HAL_TIM_CLEAR_FLAG(&htim6, TIM_FLAG_UPDATE);\
									TIM6->ARR = (period);\
									__HAL_TIM_ENABLE(&htim6)


#define DISABLE_TIMER6()		__HAL_TIM_DISABLE(&htim6);\
								__HAL_TIM_SET_COUNTER(&htim6, 0)


#define ENABLE_TIMER15()		__HAL_TIM_CLEAR_FLAG(&htim15, TIM_FLAG_UPDATE);\
								__HAL_TIM_ENABLE(&htim15)

#define TIMER15_5S				49999
#define TIMER15_1_3S			12999
#define TIMER15_1_4S			13999
#define TIMER15_1S				9999
#define TIMER15_0_3S			2999
//检出错误：3999之前写成了4999导致计算出的下一时间已经过了定时器到时的
#define TIMER15_0_4S			3999
#define TIMER15_0_8S			7999
#define TIMER15_0_9S			8999

#define TIMER4_50MS				500

#define ENABLE_TIMER15_ARR(period)		__HAL_TIM_CLEAR_FLAG(&htim15, TIM_FLAG_UPDATE);\
										TIM15->ARR = period;\
										__HAL_TIM_ENABLE(&htim15)

#define DISABLE_TIMER15()		__HAL_TIM_DISABLE(&htim15);\
								__HAL_TIM_SET_COUNTER(&htim15, 0)


#define ENABLE_TIMER4()				__HAL_TIM_CLEAR_FLAG(&htim4, TIM_FLAG_UPDATE);\
									__HAL_TIM_ENABLE(&htim4)


#define DISABLE_TIMER4()		__HAL_TIM_DISABLE(&htim4);\
								__HAL_TIM_SET_COUNTER(&htim4, 0)

//那之前的那个，还会这样出现吗？ 就是还是按照之前的值呢？
#define ENABLE_TIMER4_ARR(period)		__HAL_TIM_CLEAR_FLAG(&htim4, TIM_FLAG_UPDATE);\
										TIM4->ARR =period;\
										__HAL_TIM_ENABLE(&htim4)

/* USER CODE END Private defines */

void MX_TIM4_Init(void);
void MX_TIM6_Init(void);
void MX_TIM15_Init(void);

/* USER CODE BEGIN Prototypes */

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __TIM_H__ */

