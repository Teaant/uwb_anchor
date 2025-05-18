/*
 * uwb_mac.c
 *
 *  Created on: Sep 5, 2024
 *      Author: 24848
 */


#include <uwb_mac.h>
#include "task_manager.h"
#include "tim.h"


#define 	MAX_TXCB	10

TxDoneCb txDoneCallbacks[MAX_TXCB];


volatile UWB_Node_t uwb_node = {

		.id = MY_ID,
		.pan_id = PAN_ID,
		.uwb_phy_init = uwbInit,
		.state = initial,

};

void start_run(void) {

#if(MY_ROLE == ANCHOR)  //ANCHOR
#if(ENABLE_SYNC)  //sync

	UWB_ENABLE_RX(&uwb_node.device->ports[0]);
	ENABLE_TIMER15_ARR(TIMER15_5S);

#else   //no sync
	issue_beacon(0);   //直接开始 ？ ~
#endif  //end SYNC

#else //TAG
	//开启接收 ）
	UWB_ENABLE_RX(&uwb_node.device->ports[0]);
#endif  //end MY_ROLE
}

uint8_t initNode(void){

	uwb_node.uwb_phy_init(uwb_node.id);

	for(int i = 0 ; i< MAX_TXCB; i++) txDoneCallbacks[i] = NULL;

#if(MY_ROLE == ANCHOR)
	initAnchor();
#else
	initTag();
#endif
	return 0;
}


int register_tx_cb(uint8_t num, TxDoneCb cb){
	if(txDoneCallbacks[num] != NULL) return -1;
	txDoneCallbacks[num] = cb;
	return 1;
}




