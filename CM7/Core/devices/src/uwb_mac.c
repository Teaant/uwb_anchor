/*
 * uwb_mac.c
 *
 *  Created on: Sep 5, 2024
 *      Author: 24848
 */


#include "uwb_mac.h"
#include "uwb_mac_anchor.h"
#include "uwb_mac_tag.h"



static volatile uint32_t uwb_tick = 0;   //或可估计当前的一个值

//把Tag和anchor写成分别的两个文件
volatile UWB_Node_t uwb_node = {

		.id = MY_ID,
		.pan_id = PAN_ID,
		.interval = 2,
		.uwb_phy_init = uwbInit,

};


Timer_t my_timer = {0,};


uint8_t initNode(uint8_t role, TIM_HandleTypeDef* htim){

	uwb_node.role = role;
	my_timer.htim = htim;
	uwb_node.uwb_phy_init(uwb_node.id, uwb_node.role);   //先硬件初始化

	#if(RANGING_ROLE == ANCHOR)
		initAnchor();  //可能有一些全局变量相关的初始化  链表
	#else
		initTag();
	#endif
	return 0;
}

void Inc_Uwb_Tick(void){

	uwb_tick += 1;
//	uwb_check_state();
}

void Reset_Timer(void) {

	//失能比较输出中断
#if(RANGING_ROLE == TAG)
	DISABLE_COMP(my_timer.htim);
#else
	DISABLE_COMP1(my_timer.htim);
#endif
	// 停止定时器
	my_timer.htim->Instance->CR1 &= ~TIM_CR1_CEN;   // 禁用定时器
	// 清除计数值
	my_timer.htim->Instance->CNT = 0;
//	// (可选) 清除更新标志
//	TIM2->SR &= ~TIM_SR_UIF;
	// 重新启动定时器
	my_timer.htim->Instance->CR1 |= TIM_CR1_CEN;    // 启用定时器
}

uint16_t get_now_microSlot(void){

	return (my_timer.htim->Instance->CNT)/MICRO_TB_NUM;

}

#if(RANGING_ROLE == TAG)
void Tag_Set_Compare(uint32_t next_compare, compare_callback callback){
	//设置pulse
	my_timer.comp_value = next_compare;
	my_timer.htim->Instance->CCR2 = my_timer.comp_value;
	my_timer.callback = callback;
	ENABLE_COMP(my_timer.htim);
}

void Tag_Set_Waiting(uint32_t delta_time, compare_callback callback){

	my_timer.comp_value += delta_time;
	my_timer.htim->Instance->CCR2 = my_timer.comp_value;
	my_timer.callback = callback;
	//使能中断
	ENABLE_COMP(my_timer.htim);
}

//标签用于休眠的定时计数器
void Tag_Set_GotoSleep(uint32_t wakeup_time){

	my_timer.htim->Instance->CCR3 = wakeup_time;
	UWB_DISABLE_RX(&uwb_node.device->ports[0]);
	//使能中断
	ENABLE_COMP3(my_timer.htim);

}
//标签用于监听锚节点存在的定时器
void Tag_Start_Monitor(void){

	my_timer.htim->Instance->CCR4 = uwb_node.interval * 3 * SUPERFRAME_TB_NUM;
	ENABLE_COMP4(my_timer.htim);

}


#else

//奥对，这个的通道是不一样的
void Anchor_Set_Compare(uint32_t next_compare, compare_callback callback){
	//设置pulse
	my_timer.comp_value = next_compare;
	my_timer.htim->Instance->CCR1 = my_timer.comp_value;
	my_timer.callback = callback;
	ENABLE_COMP1(my_timer.htim);
}

void Anchor_Set_CompareTag(uint8_t tag_index, uint32_t next_compare, anchor_tag_callback callback, uint8_t param){

	switch(tag_index){
	case 1:
		my_timer.htim->Instance->CCR2 = next_compare;
		my_timer.callback1 = callback;
		my_timer.param1 = param;
		ENABLE_COMP(my_timer.htim);
		break;
	case 2:
		my_timer.htim->Instance->CCR3 = next_compare;
		my_timer.callback2 = callback;
		my_timer.param2 = param;
		ENABLE_COMP3(my_timer.htim);
		break;
	case 3:
		my_timer.htim->Instance->CCR4 = next_compare;
		my_timer.callback3 = callback;
		my_timer.param3 = param;
		ENABLE_COMP4(my_timer.htim);
		break;
	default:
		break;
	}
}


void Anchor_Stop_CompareTag(uint8_t tag_index){

	switch(tag_index){
	case 1:
		DISABLE_COMP(my_timer.htim);
		break;
	case 2:
		DISABLE_COMP3(my_timer.htim);
		break;
	case 3:
		DISABLE_COMP4(my_timer.htim);
		break;
	}

}

#endif

//static void uwb_handle_ack(UWB_Ranging_ACK_t* pdata){
//	float distance;
//	int64_t tof_dtu = pdata->tof_dtu;
//	distance = (float) tof_dtu * DWT_TIME_UNITS * SPEED_OF_LIGHT;
//	//或者其实也可以直接把
//	if (distance < 0.3) {
//		distance = 0.3;
//	}
//	//Tanya_add keep the distance calculate
//	//	uwb_distance_filter(&Ranging.ranging_node, distance);
//
//}
///**
// * poll tx resp rx 时间戳
// * 发送final帧
// */
//static void uwb_handle_resp(void){
//
//	/**
//	 * @TODO  handle resp
//	 * 还需要再比对一些东西，比如sequence之类的
//	 */
//	uint32 final_tx_time;
//	uwb_node.ranging_values.poll_tx_ts = get_tx_timestamp_u64(&(uwb_node.device->ports[0]));
//	uwb_node.ranging_values.resp_rx_ts = get_rx_timestamp_u64(&(uwb_node.device->ports[0]));
//	/* Compute final message transmission time. See NOTE 10 below. */
//	final_tx_time = (uwb_node.ranging_values.resp_rx_ts + (RESP_RX_TO_FINAL_TX_DLY_UUS * UUS_TO_DWT_TIME)) >> 8;
//
//	uwb_node.ranging_values.final_tx_ts = (((uint64)(final_tx_time & 0xFFFFFFFEUL)) << 8) + uwb_node.device->antDelay; //时间戳
//
//	packFinal(uwb_node.txBuffer);
//	//send
//	UWB_SEND_RANGING_FINAL(uwb_node.txBuffer, final_tx_time);
//
//}
//
//
//
//
////ThisMsg is no need anymore
//static void packAck(uint8* pbuffer){
//
//}



