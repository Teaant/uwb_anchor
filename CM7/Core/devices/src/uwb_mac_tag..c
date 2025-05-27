/*
 * uwb_mac_tag..c
 *
 *  Created on: Sep 28, 2024
 *      Author: 24848
 */

#include "uwb_mac.h"
#include "tim.h"
#include "uwb_mac_tag.h"
#include "task_manager.h"

#include "../../../util/mem_manager.h"

#if(MY_ROLE == TAG)


UWB_RangingValue_t ranging_anchor_values;

static slot_alloc_node_t* anchor_table;

extern UWB_Node_t uwb_node;


static void remove_anchor_from_table(uint16_t id);
static slot_alloc_node_t* find_anchor_in_table(uint16_t id);
static slot_alloc_node_t* hear_an_anchor(uint16_t id, uint16_t pan_id);


static void applying_txDone_cb(uint64_t tx_ts);
static void poll_txDone_cb(uint64_t tx_ts);
static void final_txDone_cb(uint64_t tx_ts);

static void request_timeout(void);
static void resp_timeout(void);

static void wakeup_tag(void);

volatile Tag_Struct_t tag_struct= {
		.timer15_callback = wakeup_tag,
};

extern Flash_Config_t	flash_config;

void initTag(void){

	uwb_node.state = initial;
	uwb_node.ptag_struct = &tag_struct;

	tag_struct.interval = flash_config.tag_interval;
	//wake_up time
	//TIM15的时钟频率
	uwb_node.wakeup_time = (tag_struct.interval -1) * 10000 + 9000;   //若是很大的呢？ 暂时只考虑6以内？  900ms行哈，有100ms的雨量，那我就150ms

	register_tx_cb(applying, applying_txDone_cb);
	register_tx_cb(polling, poll_txDone_cb);
	register_tx_cb(finaling, final_txDone_cb);


}

static void update_interval(uint16_t interval){

	flash_config.tag_interval = interval;
	if(Flash_WriteConfig(&flash_config) == HAL_OK){
		Software_Reset();
	}else{
		//keep Rxing
		UWB_ENABLE_RX(&uwb_node.device->ports[0]);
	}
}

static void update_tx_power(uint16_t power){

	uint8_t interval = (uint8_t)power;

	flash_config.tx_power = (uint32_t)interval << 24 | (uint32_t)interval << 16
            | (uint32_t)interval << 8  | interval;
	if(Flash_WriteConfig(&flash_config) == HAL_OK){
		Software_Reset();
	}else{
		//keep Rxing
		UWB_ENABLE_RX(&uwb_node.device->ports[0]);
	}
}

void tag_parse_ranging(uint64_t rx_ts) {

	UWB_Msg_Header_t *pheader = (UWB_Msg_Header_t*) tag_struct.rxBuffer; //接收数据包的第一个字节当前
	//解析头部信息
	uint8_t frame_type = GET_FRAMETYPE(pheader->control);

	if (pheader->dist != MY_ID && pheader->dist != 0xFFFF)
		return ;

	switch (frame_type) {
	case beacon_frame:
#if(ENABLE_SYNC)
		UWB_Sync_Header_t * sync_header = (UWB_Sync_Header_t*)(tag_struct.rxBuffer + UWB_MAC_HEADER_LEN);
		uint16_t* pID = sync_header->Slots;
		uint8_t bop ;
		for(bop = 0; bop < 4; bop++){
			if(*(pID+bop) == pheader->src) {
				break;
			}
		}
		//计算Start of active
		if(bop == 1 || bop == 3){
			tag_struct.beacon_start_time = getDeltaT(rx_ts, MS_2);
		}else {
			tag_struct.beacon_start_time = rx_ts;
		}
#endif
		UWB_Beacon_Payload_t *pbeacon = (UWB_Beacon_Payload_t*) (tag_struct.rxBuffer + UWB_MSG_COMMON_LEN);

		if(uwb_node.state == initial){
			tag_struct.cap_start = pbeacon->sCAP;
			tag_struct.applying_anchor = pheader->src;
			tag_struct.applying_anchor_pan = pheader->pan_id;
			enqueueTask(prepare_join, pheader->src);
		}else{
			uwb_handle_beacon(pheader->src);
		}
		break;
	case data_frame:
		UWB_Date_Resp_t * pdata = (UWB_Date_Resp_t*) tag_struct.rxBuffer;
		switch (pdata->function){
		case UWB_Ranging_Resp:
			if (pheader->src == ranging_anchor_values.pnode->slot_alloc.node_id && uwb_node.state == polling) {
				uint16_t *pID = &pdata->ID1;
				for (int i = 0; i < 3; i++) {
					if (*(pID + i) == MY_ID) {
						DISABLE_TIMER6();  //停止resp_timeout  //应该是远远还没到的？
						//schedule the final msg ~
						ranging_anchor_values.resp_rx_ts = rx_ts;
						enqueueTask(prepare_final, pheader->src);
						return;
					}
				}
			}
			UWB_ENABLE_RX(&uwb_node.device->ports[0]);
			break;
		default:
			break;
		}
		break;
	case ack_frame:
		if(pheader->src == tag_struct.applying_anchor && uwb_node.state == applying && pheader->sequence == uwb_node.sequence){
			//stop the timer  ~
			DISABLE_TIMER6();
			slot_alloc_node_t* panchor = find_anchor_in_table(pheader->src);
			panchor->slot_alloc.miss = 0;
			panchor->slot_alloc.pan_id = pheader->pan_id;
			ranging_anchor_values.pnode = panchor;
			uwb_node.state = listening;
		}
		//keep RXing
		UWB_ENABLE_RX(&uwb_node.device->ports[0]);
		break;
	case mac_cmd_frame:
		//MAC控制帧
		UWB_Mac_Payload_t * pmac = (UWB_Mac_Payload_t*)(tag_struct.rxBuffer + sizeof(UWB_Msg_Header_t));
		switch(pmac->function){
		case UWB_Cmd_Interval:
			enqueueTask(update_interval, (uint16_t)pmac->interval);
			break;
		case UWB_Cmd_Power:
			enqueueTask(update_tx_power, (uint16_t)pmac->interval);
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}



void prepare_join(uint16_t id){

	//根据自己的ID选择一个ALOHA时隙发送请求
	uint8_t random_slot = MY_ID % ((249 - tag_struct.cap_start)/2-1) + 1;

	UWB_Mac_Frame_t* joing_msg = (UWB_Mac_Frame_t*)tag_struct.txBuffer;

	joing_msg->header.control = MAC_CMD_CONTROL;
	joing_msg->header.sequence = ++ uwb_node.sequence;
	joing_msg->header.pan_id = tag_struct.applying_anchor_pan;
	joing_msg->header.dist = id;
	joing_msg->header.src = MY_ID;

	joing_msg->payload.function = UWB_Cmd_Req;
	joing_msg->payload.interval = uwb_node.ptag_struct->interval;
	//schedule
	uint64_t delatT = (uint64_t)MS_2 * (tag_struct.cap_start + 2 * random_slot);
	uint64_t req_tx_time = getSumT(tag_struct.beacon_start_time, delatT);

	UWB_Tag_Req_Join((uint8_t*)tag_struct.txBuffer, REQ_ACK_MSG_LEN, req_tx_time&TX_TIME_MASK);

	uwb_node.state = applying;
	hear_an_anchor(tag_struct.applying_anchor, tag_struct.applying_anchor_pan);

}

//还是可以设置这个的CB呢？
//设置TIMER2吧
static void applying_txDone_cb(uint64_t tx_ts){

	ENABLE_TIMER6_ARR(TIMER6_4MS);  //应该是4MS
	//设置Callback
	tag_struct.timer6_callback = request_timeout;

}


static void request_timeout(void){
	if(uwb_node.state == applying) {
		uwb_node.state = initial;
	}
}


void uwb_handle_beacon(uint16_t id){

	UWB_Beacon_Payload_t *pbeacon = (UWB_Beacon_Payload_t*)(tag_struct.rxBuffer + UWB_MSG_COMMON_LEN);

	uint16_t* pTags = pbeacon->IDs;
	//是当前的测距的anchor
	if(ranging_anchor_values.pnode->slot_alloc.node_id == id){
		//stop the absence timer ~
		DISABLE_TIMER4();
		for(int i = 0; i< pbeacon->Ntags; i++){
			if(*(pTags+i) == uwb_node.id){
				ranging_anchor_values.pnode->slot_alloc.miss = 0; //清除不良记录
				tag_struct.macro = i+1;
				tag_struct.micro1 = GET_MICRO_SLOT1(i+1);
				tag_struct.poll_tx_time = getSumT(tag_struct.beacon_start_time, MS_2 * tag_struct.micro1);
//				tag_struct.poll_tx_time &= TX_TIME_MASK;
				prepare_poll(id);
				ENABLE_TIMER15_ARR(uwb_node.wakeup_time);	//set for the next ranging
				return;
			}
		}
		ranging_anchor_values.pnode->slot_alloc.miss ++;
		if(ranging_anchor_values.pnode->slot_alloc.miss > tag_struct.interval + 2){
			//outside了
			uwb_node.state = initial;
			ranging_anchor_values.pnode->slot_alloc.miss = 0;
		}else{
			ENABLE_TIMER4();  //still restart timer_4，那就不是anchor_absent ?
		}
		UWB_ENABLE_RX(&uwb_node.device->ports[0]);
	}else{
		hear_an_anchor(id, PAN_ID);
		UWB_ENABLE_RX(&uwb_node.device->ports[0]);
	}
}

/**
 * Done
 */
void prepare_poll(uint16_t id){

	UWB_Date_Poll_t* ppoll = (UWB_Date_Poll_t*)(tag_struct.txBuffer);
	ppoll->header.control = POLL_CONTROL;
	ppoll->header.dist = ranging_anchor_values.pnode->slot_alloc.node_id;
	ppoll->header.pan_id = ranging_anchor_values.pnode->slot_alloc.pan_id;
	ppoll->header.src = MY_ID;
	ppoll->header.sequence = ++uwb_node.sequence;
	ppoll->function = UWB_Ranging_Poll;
	//Schedule the msg~
	uwb_node.state = polling;
	UWB_Tag_Send_Ranging((uint8_t*)ppoll, POLL_MSG_LEN, tag_struct.poll_tx_time);

	uint64_t time = get_sys_timestamp(&uwb_node.device->ports[0]);

}

void prepare_final(uint16_t id){

	UWB_Data_Final_t* pfinal = (UWB_Data_Final_t*)(tag_struct.txBuffer);
	pfinal->header.control = FINAL_CONTROL;
	pfinal->header.dist = ranging_anchor_values.pnode->slot_alloc.node_id;
	pfinal->header.pan_id = ranging_anchor_values.pnode->slot_alloc.pan_id;
	pfinal->header.src = MY_ID;
	pfinal->header.sequence = uwb_node.sequence;
	pfinal->function = UWB_Ranging_Final;

	pfinal->final_payload.poll_tx_ts = ranging_anchor_values.poll_tx_ts;
	pfinal->final_payload.resp_rx_ts = ranging_anchor_values.resp_rx_ts;
	//这个果然是有一点影响的
	pfinal->final_payload.final_tx_ts = getSumT(tag_struct.final_tx_time, uwb_node.device->antDelay);

	//Schedule the msg~
	uwb_node.state = finaling;
	UWB_Tag_Send_Ranging((uint8_t*)pfinal, FINAL_MSG_LEN, tag_struct.final_tx_time);

}

static void after_poll(uint16_t id) {

//	int ret;

//	uint64_t now_time = get_sys_timestamp(&uwb_node.device->ports[0]);

//	ret = dwt_rxenable(DWT_START_RX_IMMEDIATE, &uwb_node.device->ports[0]); //为啥会失败啊？怎么回事了？

//	if (ret != DWT_SUCCESS) {
//		rx_fail++;
//	}
	UWB_ENABLE_DELAY_RX(1000, &uwb_node.device->ports[0]);
	tag_struct.timer6_callback = resp_timeout;
	tag_struct.micro3 = GET_MICRO_SLOT3(tag_struct.macro);//这需要很久吗？
	tag_struct.final_tx_time = getSumT(tag_struct.beacon_start_time, MS_2 * tag_struct.micro3);
	tag_struct.final_tx_time &= TX_TIME_MASK;

}
static void poll_txDone_cb(uint64_t tx_ts){


	ENABLE_TIMER6_ARR(TIMER6_12MS);
	//0.234ms    从发送结束到处理到这里
	ranging_anchor_values.poll_tx_ts = tx_ts;
	enqueueTask(after_poll, 0);

}
//过在进入了resp_timeout ?
static void resp_timeout(void){

	//看下当前时间
	//但是当下的时间，这个时间好像不对啊？
	uint64_t now_time = get_sys_timestamp(&uwb_node.device->ports[0]);
	uwb_node.state = sleeping;   //当前应该是处于RX模式的啊？为啥?什么个事？  //为什么3ms就没有了？ 是那么情况哎？
	UWB_DISABLE_RX(&uwb_node.device->ports[0]);

}

static void wakeup_tag(void){

	uwb_node.state  = listening;
	UWB_ENABLE_RX(&uwb_node.device->ports[0]);
	ENABLE_TIMER4();
}

static void final_txDone_cb(uint64_t tx_ts){
	//go to sleep
	uwb_node.state = sleeping;
	UWB_DISABLE_RX(&uwb_node.device->ports[0]);
}


static slot_alloc_node_t* find_anchor_in_table(uint16_t id){
	slot_alloc_node_t* panchor = anchor_table;
	while(panchor != NULL){
		if(panchor->slot_alloc.node_id == id){
			break;
		}else{
			panchor = panchor->pnext;
		}
	}
	return panchor;
}

static slot_alloc_node_t* hear_an_anchor(uint16_t id, uint16_t pan_id){
	slot_alloc_node_t* panchor  = find_anchor_in_table(id);
	if(panchor == NULL){
		panchor = memp_alloc();
		if(panchor){
			//还要考虑链表是空的情况
			panchor->slot_alloc.node_id = id;
			panchor->slot_alloc.pan_id = pan_id;
			panchor->slot_alloc.absence =0;
			panchor->slot_alloc.miss =0;
			if(anchor_table){
				panchor->pnext = anchor_table;
			}
			//如果table是空，直接指向即可
			anchor_table = panchor;
			return panchor;
		}else{
			return NULL;
		}
	}
	return panchor;
}

static void remove_anchor_from_table(uint16_t id){
	slot_alloc_node_t* p, *q;
	p = anchor_table;
//第一个需要特殊考虑
	if(anchor_table->slot_alloc.node_id == id){
		anchor_table = anchor_table->pnext;
		memp_free(p);
	}else{
		while(p->pnext != NULL){
			if(p->pnext->slot_alloc.node_id == id){
				q = p->pnext;
				p->pnext = p->pnext->pnext; //指向下一个了
				memp_free(q);
				return;
			}else{
				p = p->pnext;
			}

		}
	}
}

void Tag_lose_anchor(){
	//移除
	remove_anchor_from_table(ranging_anchor_values.pnode->slot_alloc.node_id);
	//状态切换
	uwb_node.state = initial;
	//开启接收
	UWB_ENABLE_RX(&uwb_node.device->ports[0]);
}

/**
 * 1. 计算信号强度
 * 2. 记录有效性，准备定位通信
 * 3. 记录ranging_value
 * valid_anchor(uwb_node.header.src);
 */
void valid_anchor(uint16_t id){

	slot_alloc_node_t* panchor = find_anchor_in_table(id);
	panchor->slot_alloc.signal = uwb_calculate_power(&uwb_node.device->ports[0]);  //这个结果为什么是0 ？
	panchor->slot_alloc.absence = 0;

}


#endif


