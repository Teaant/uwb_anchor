/*
 * uwb_mac.h
 *
 *  Created on: Sep 5, 2024
 *      Author: 24848
 */

#ifndef DEVICES_INC_UWB_MAC_H_
#define DEVICES_INC_UWB_MAC_H_

#include <uwb_consts.h>
#include "uwb.h"
#include "uwb_ranging.h"
#include "uwb_msg.h"
#include "agent.h"
#include "corecomm.h"

#if(MY_ROLE == ANCHOR)
#include "uwb_mac_anchor.h"
#else
#include "uwb_mac_tag.h"
#endif


#define RX_TEST	0

#define	TX_MODE	1

#define UWB_ENABLE_DELAY_RX(delay,pport)	dwt_setrxaftertxdelay(delay, pport);\
											dwt_rxenable(DWT_START_RX_DELAYED, pport)

#define UWB_ENABLE_RX(pport)	 dwt_rxenable(DWT_START_RX_IMMEDIATE, pport)
//关闭接收，进入idle模式
#define UWB_DISABLE_RX(pport)	dwt_forcetrxoff(pport); \
								dwt_rxreset(pport)


#define DS_TWR_TIMES			3
#define GET_MICRO_SLOT1(macro)  (((((macro)-1)/3)*(9) +((macro)-1)%3+1)+1)
#define GET_MICRO_SLOT2(macro)  ((GET_MICRO_SLOT1(macro))+3)
#define GET_MICRO_SLOT3(macro)  ((GET_MICRO_SLOT1(macro))+6)

typedef enum{
	//Anchor
	initial = 0,

	beaconing,
	ranging,
	non_ranging,

	//标签似乎不太需要的
	applying,
	listening,
	sleeping,
	polling,
	finaling,

}Node_State_t;  // 只需要一个状态


typedef struct{

	uint8_t if_switch;
	uint16_t node_id;
	uint16_t pan_id; //感觉pan_id也不是说必需的
	float signal;		//信号强度
#if(MY_ROLE == ANCHOR)
	uint8_t  is_valid;		//是否有效
	uint8_t  macro;			//宏时隙
	uint16_t micro1;		//微时隙1
	uint16_t micro2;
	uint16_t micro3;
	uint8_t  interval;  	//定位周期
	uint8_t  time_to_locate;   //-1  every superframe  锚节点遍历该值确定是否测距标签节点
#endif
	uint8_t absence;		//缺席次数
	uint8_t miss;
	uint16_t times;    //测距次数   	 一个记录而已

}Slot_Alloc_t;

typedef struct Slot_Item{
	Slot_Alloc_t slot_alloc;
	struct Slot_Item* pnext;
}slot_alloc_node_t;


typedef struct{

	slot_alloc_node_t* pnode;

	uint64_t poll_tx_ts;
	uint64_t resp_rx_ts;
	uint64_t final_tx_ts;

#if(MY_ROLE == ANCHOR)
	//anchor
	uint8_t sequence;
	uint8_t is_valid;
	uint8_t is_available;   //测距有效性
	uint8_t absence;
	uint64_t poll_rx_ts;
	uint64_t resp_tx_ts;
	uint64_t final_rx_ts;
	int64_t R1, R2, D1, D2;

	float distance;
#endif

}UWB_RangingValue_t;

typedef struct{

	uint16_t pan_id;
	uint16_t id;

	volatile Node_State_t state;   //状态 ~

	uint8_t sequence;
	uint8_t rand_ok;

	UWBDef* device;

//	UWB_Msg_Header_t header;
	uint16_t wakeup_time;  //in ms , I do not know you know ,this is going to do what then ？

#if(MY_ROLE == ANCHOR)
	volatile Anchor_Struct_t* panchor_struct;
#else
	volatile Tag_Struct_t* ptag_struct;
#endif

	int32_t (*uwb_phy_init)(uint16_t ID);
	uint32_t (*get_rand)(void);  //获取一个随机数

}UWB_Node_t;


typedef void(*TxDoneCb)(uint64_t tx_ts);

uint8_t initNode(void);

void start_run(void);

int register_tx_cb(uint8_t num, TxDoneCb cb);

#endif /* DEVICES_INC_UWB_MAC_H_ */
