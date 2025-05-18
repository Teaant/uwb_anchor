/*
 * msg_queue.h
 *
 *  Created on: May 4, 2025
 *      Author: 24848
 */

#ifndef MSG_QUEUE_H_
#define MSG_QUEUE_H_

#include "main.h"

//单纯从锚节点角度来

#define MAX_MSG_READ_LEN	40

typedef struct{
	uint64_t rx_ts;
	uint8_t	rcv_data[MAX_MSG_READ_LEN];
}UWB_Rcv_Msg_t;

UWB_Rcv_Msg_t* req_msg_buffer(void);

int release_msg_buffer(UWB_Rcv_Msg_t* msg);

UWB_Rcv_Msg_t* get_msg_from_queue(void);

int put_msg_in_queue(UWB_Rcv_Msg_t* msg);


#endif /* MSG_QUEUE_H_ */
