/*
 * task_manager.h
 *
 *  Created on: Sep 28, 2024
 *      Author: 24848
 */

#ifndef UTILES_INC_TASK_MANAGER_H_
#define UTILES_INC_TASK_MANAGER_H_

#include "main.h"

typedef  void (*U_Task)(uint16_t id);



uint8_t enqueueTask(U_Task task, uint16_t id);
U_Task dequeueTask(uint16_t * param);
void empty_fifo(void);
uint8_t getTaskNum(void);


#endif /* UTILES_INC_TASK_MANAGER_H_ */
