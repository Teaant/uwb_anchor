/*
 * task_manager.c
 *
 *  Created on: Sep 28, 2024
 *      Author: 24848
 */

#include "task_manager.h"

#define	QUEUE_SIZE	50

U_Task tasks_fifo[QUEUE_SIZE];
uint16_t params_fifo[QUEUE_SIZE];
uint8_t front = 0, rear;


uint8_t enqueueTask(U_Task task, uint16_t id){
	//队列满
	if((rear+1)%QUEUE_SIZE == front){
		return 0;
	}else{
		tasks_fifo[rear] = task;
		params_fifo[rear] = id;
		rear = (rear+1)%QUEUE_SIZE;
		return 1;
	}
}

U_Task dequeueTask(uint16_t * param){
	//队列空
	U_Task re_task;  //怎么几次之后卡在这里了吗？

	if(front == rear){
		return NULL;
	}else{
		*param = params_fifo[front];
		re_task = tasks_fifo[front];
		front = (front+1)%QUEUE_SIZE;
		return re_task;
	}
}

void empty_fifo(void){
	front = rear;
}

uint8_t getTaskNum(void){
	return ((rear- front + QUEUE_SIZE)%QUEUE_SIZE);
}
