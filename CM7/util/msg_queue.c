/*
 * msg_queue.c
 *
 *  Created on: May 4, 2025
 *      Author: 24848
 */

#include "msg_queue.h"

volatile static uint32_t alloc_mask = 0;
volatile static UWB_Rcv_Msg_t 	msg_pool[32];

volatile static uint8_t alloc_pos = 0;

#define	QUEUE_SIZE	32

volatile static UWB_Rcv_Msg_t* msg_fifo[QUEUE_SIZE];

volatile static uint8_t front = 0, rear;


static int8_t find_free_pos(void){
	uint8_t pos = alloc_pos + 1;   //从上一次已经分配出去的位置开始 ~
	uint8_t k ;
	for(k = 0; k < 32; k ++){
		pos = (pos + k)%32;  //避免溢出 ~
		if((alloc_mask&(0x00000001<<pos)) == 0){
			alloc_pos = pos;
			return pos;
		}
	}
	alloc_pos = -1;
	return -1;
}

UWB_Rcv_Msg_t* req_msg_buffer(void){

	int8_t pos = find_free_pos();
	if(pos >= 0){
		alloc_mask &= 0x00000001 << pos;
		return &msg_pool[pos];
	}else {
		return NULL;
	}

}

int release_msg_buffer(UWB_Rcv_Msg_t* msg){

	uint8_t pos = msg - msg_pool;
	memset((uint8_t*)(&msg_pool[pos]), 0, sizeof(UWB_Rcv_Msg_t));
	alloc_mask &= ~(0x00000001<<pos);

}

UWB_Rcv_Msg_t* get_msg_from_queue(void){

	UWB_Rcv_Msg_t* msg = NULL;
	//队列空
	if (front == rear) {
		return NULL;
	} else {
		msg = msg_fifo[front];
		front = (front + 1) % QUEUE_SIZE;
		return msg;
	}

}

int put_msg_in_queue(UWB_Rcv_Msg_t *msg) {

	if ((rear + 1) % QUEUE_SIZE == front) {
		return 0;
	} else {
		msg_fifo[rear] = msg;
		rear = (rear + 1) % QUEUE_SIZE;
		return 1;
	}

}
