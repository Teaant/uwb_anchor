/*
 * uwb_mac.h
 *
 *  Created on: Sep 5, 2024
 *      Author: 24848
 */

#ifndef DEVICES_INC_UWB_MAC_H_
#define DEVICES_INC_UWB_MAC_H_

#include "uwb.h"
#include "uwb_ranging.h"
#include "uwb_msg.h"
#include "uwb_consts.h"

//定义角色
#define  	RANGING_ROLE    	ANCHOR
#define 	Tanya_Test			1

#if(RANGING_ROLE == TAG)
#define 	MY_ID				0xBBBB
#else
#define 	MY_ID				0xAAAA
#endif
#define 	PAN_ID				0x3737

#if(RANGING_TAG == ANCHOR)
#include "uwb_mac_anchor.h"
#else
#include "uwb_mac_tag.h"
#endif

/**
 * k = ((macro-1)/3)*(3 * 3) +(macro-1)%3+1;
   printf("宏时隙：%d, 微时隙：%d, %d, %d, %d\n", macro, k, k+3, k+6, k+9);
 *
 */
#define DS_TWR_TIMES			3
#define GET_MICRO_SLOT1(macro)  ((((macro)-1)/UWB_REPLY_INTERVAL)*(UWB_REPLY_INTERVAL*DS_TWR_TIMES) +((macro)-1)%UWB_REPLY_INTERVAL+1)
#define GET_MICRO_SLOT2(macro)  ((GET_MICRO_SLOT1(macro))+UWB_REPLY_INTERVAL)
#define GET_MICRO_SLOT3(macro)  ((GET_MICRO_SLOT1(macro))+UWB_REPLY_INTERVAL*2)
//#define GET_MICRO_SLOT4(macro)  ((GET_MICRO_SLOT1(macro))+UWB_REPLY_INTERVAL*3)


#define  PDoA_RX_Buffer  UWB_Msg_Header_t

//使能接收
#define UWB_ENABLE_RX(pport)	 dwt_rxenable(DWT_START_RX_IMMEDIATE, pport)
//关闭接收，进入idle模式
#define UWB_DISABLE_RX(pport)	dwt_forcetrxoff(pport); \
								dwt_rxreset(pport)

/**
 * void UWB_Send(uint8_t * pdata, uint8_t len, If_Delay_t is_delayed, uint32_t tx_time, If_Expected_t is_expect)
 */
#define UWB_SEND_RANGING_POLL(pdata)				UWB_Send(pdata, POLL_MSG_LEN, 0, 0, 1)
#define UWB_SEND_RANGING_RESP(pdata, tx_time)		UWB_Send(pdata, RESP_MSG_LEN, 1, tx_time, 1)
#define UWB_SEND_RANGING_FINAL(pdata, tx_time)		UWB_Send(pdata, FINAL_MSG_LEN, 1, tx_time, 0)
#define UWB_SEND_RANGING_ACK(pdata, tx_time)		UWB_Send(pdata, ACK_MSG_LEN, 1, tx_time, 0)

#define UWB_SEND_JOIN_REQ(pdata)					UWB_Send(pdata, JOIN_REQ_MSG_LEN, 0, 0, 1)

//anchor的main函数中遍历slot_alloc_table,准备好Beacon帧

typedef struct{
	uint16_t node_id;
	uint16_t pan_id; //感觉pan_id也不是说必需的
	float signal;		//信号强度
	uint8_t  macro;			//宏时隙
	uint8_t  is_valid;		//是否有效
	uint16_t micro1;		//微时隙1
	uint16_t micro2;
	uint16_t micro3;
//	uint16_t micro4;
	uint8_t  interval;  	//定位周期
	uint8_t  time_to_locate;   //-1  every superframe  锚节点遍历该值确定是否测距标签节点
	uint8_t absence;		//缺席次数
	uint8_t miss;
//#if(RANGING_ROLE == ANCHOR)  //共享内存？
//	float distance;
//#endif

}Slot_Alloc_t;

typedef struct Slot_Item{
	Slot_Alloc_t slot_alloc;
	struct Slot_Item* pnext;
}slot_alloc_node_t;  //20B?


typedef struct{

	uint64 poll_tx_ts;
	uint64 resp_rx_ts;
	uint64 final_tx_ts;
#if(RANGING_ROLE == TAG)
	slot_alloc_node_t * panchor;
#else
	//anchor
	slot_alloc_node_t * ptag;
	uint8_t sequence;
	uint8_t is_valid;
	uint8_t is_available;   //测距有效性
	uint64 poll_rx_ts;
	uint64 resp_tx_ts;
	uint64 final_rx_ts;
	int64 tof_dtu;
	int64 R1, R2, D1, D2;   //其实也没什么必要弄这个，要是有的话到时候再说了，就这样吧先
	float distance;
	//其实只要修改一下id和sequence就行了，那直接用一个完全是足够的
	/**
	 * @TODO:锚节点增加角度相关记录？
	 */
#endif

}UWB_RangingValue_t;


typedef enum{
	outside =0,
	member,
}UWB_Tag_State_t;

typedef enum{

	//member
	sleeping = 0,
	listening , //wake-up from sleep waiting beacon
	ready_ranging,   //ready for the slot to send poll
	polling ,  //waiting for resp
	finaling , //waiting for ack

	//outside
	idle,		 //listening beacon
	ready_apply, //waiting for CAP to apply
	applying,    //send apply

}UWB_Tag_SubState_t;


typedef struct{
	uint16_t anchor_id;
	uint16_t pan_id;
	uint8_t my_macro_slot; //当前我的宏时隙号
	uint8_t cfp_macro_slot_num;       //当前超帧tag节点的总数  tag_now * 9 +1  CAP开始微时隙号
	uint8_t cap_macro_slot_num;
	uint8_t my_micro_slots[3];
}SuperFrame_t;

//或许需要维护一个beacon帧的内容
typedef struct{

	uint16_t pan_id;
	uint16_t id;
	//自己的interval
	UWB_Role_t role;
	uint8_t interval;
	uint8_t state;
	uint8_t sub_state;
	UWBDef* device;
	//sequence  only 1 octet
	uint8_t sequence;
//	slot_alloc_node_t*  pslot_alloc_table;

	UWB_Msg_Header_t header;
#if(RANGING_ROLE == ANCHOR)
	PDoA_RX_Buffer pdoa_buffer;
#endif
	uint8_t rxBuffer[128];       //125
	uint8_t txBuffer[128];
	int32_t (*uwb_phy_init)(uint16_t ID, UWB_Role_t role);

}UWB_Node_t;

//随机数生成，有硬件支持 ~


/*
基准时间 100us			autoReload = 100-1;  ---> 中断
时钟精度 1us             preScale ---->  1us
考虑一个实现：
2ms 时隙
500ms / 2ms = 250
250 -1 = 249 空出一个 ? 缓冲也行
249 - 50*4 = 49
49/2 = 24
最多可以容纳 这么多节点    249 / 4 = 62
 */
//24 * 4ms = 96ms

//标签节点出现了ENABLE_COMP调用之后马上就进入了中断，所以在此处我把中断标志先清除了然后再使能，之后就没再出现了
#define ENABLE_COMP1(htim)		__HAL_TIM_CLEAR_FLAG(htim, TIM_FLAG_CC1);\
								__HAL_TIM_ENABLE_IT(htim, TIM_IT_CC1)

#define DISABLE_COMP1(htim)		__HAL_TIM_DISABLE_IT(htim, TIM_IT_CC1)

#define ENABLE_COMP(htim)		__HAL_TIM_CLEAR_FLAG(htim, TIM_FLAG_CC2);\
								__HAL_TIM_ENABLE_IT(htim, TIM_IT_CC2)

#define DISABLE_COMP(htim)		__HAL_TIM_DISABLE_IT(htim, TIM_IT_CC2)


#define ENABLE_COMP3(htim)		__HAL_TIM_CLEAR_FLAG(htim, TIM_FLAG_CC3);\
								__HAL_TIM_ENABLE_IT(htim, TIM_IT_CC3)

#define DISABLE_COMP3(htim)		__HAL_TIM_DISABLE_IT(htim, TIM_IT_CC3)


#define ENABLE_COMP4(htim)		__HAL_TIM_CLEAR_FLAG(htim, TIM_FLAG_CC4);\
								__HAL_TIM_ENABLE_IT(htim, TIM_IT_CC4)

#define DISABLE_COMP4(htim)		__HAL_TIM_DISABLE_IT(htim, TIM_IT_CC4)


typedef void (*compare_callback)(void);

typedef void (*anchor_tag_callback)(uint8_t);


typedef struct{
	TIM_HandleTypeDef *htim;
	uint32_t comp_value;
	compare_callback callback;

#if(RANGING_ROLE == ANCHOR)
	anchor_tag_callback callback1;
	anchor_tag_callback callback2;
	anchor_tag_callback callback3;
	uint8_t param1;
	uint8_t param2;
	uint8_t param3;

#endif
}Timer_t;


/**
 * 根据role初始化节点结构体的参数
 */
uint8_t initNode(uint8_t role, TIM_HandleTypeDef* htim);
void Inc_Uwb_Tick(void);

void Reset_Timer(void);
uint16_t get_now_microSlot(void);


void Tag_Set_Compare(uint32_t next_compare, compare_callback callback);
void Tag_Set_Waiting(uint32_t delta_time, compare_callback callback);


void Tag_Set_GotoSleep(uint32_t wakeup_time);
void Tag_Start_Monitor(void);


void Anchor_Set_Compare(uint32_t next_compare, compare_callback callback);
void Anchor_Stop_CompareTag(uint8_t tag_index);
void Anchor_Set_CompareTag(uint8_t index, uint32_t next_compare, anchor_tag_callback callback, uint8_t param);


#endif /* DEVICES_INC_UWB_MAC_H_ */
