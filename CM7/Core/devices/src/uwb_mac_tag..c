/*
 * uwb_mac_tag..c
 *
 *  Created on: Sep 28, 2024
 *      Author: 24848
 */

#include "uwb_mac_tag.h"
#include "uwb_mac.h"

#include "task_manager.h"
#include "mem_manager.h"

#if(RANGING_ROLE == TAG)

//MICRO_TB_NUM
#define COMPARE_OFFSET	4

static uint8_t current_micro_slot = 0;

uint16_t  applying_anchor, applying_pan_id;

UWB_RangingValue_t ranging_anchor_values;


static slot_alloc_node_t* anchor_table;

static SuperFrame_t superframe= {0, };

extern UWB_Node_t uwb_node;
extern Timer_t my_timer;

static  uint32_t timer_tick;

__weak void uwb_handle_func2(uint16_t  id){

	UNUSED(id); // 避免unused警告

}

static void packPoll(uint8_t* pbuffer);
static void packFinal(uint8_t* pbuffer);
//假设之前如果没有？ 应该要有的啊
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
			panchor->slot_alloc.is_valid = 0;
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


void initTag(void){

	uwb_node.state = outside;
	uwb_node.sub_state = idle;
	UWB_ENABLE_RX(&uwb_node.device->ports[0]);  //开启接收
}

void tag_parse_ranging(uint16_t now_slot) {

	UWB_Msg_Header_t *pheader = (UWB_Msg_Header_t*) uwb_node.rxBuffer; //接收数据包的第一个字节当前
	uint8_t *ppayload = uwb_node.rxBuffer + UWB_MAC_HEADER_LEN;  //当前指向负载部分
	//解析头部信息
	uint8_t frame_type = GET_FRAMETYPE(pheader->control);
	uwb_node.header.src = pheader->src;
	uwb_node.header.dist = pheader->dist;
	uwb_node.header.pan_id = pheader->pan_id;
	uwb_node.header.sequence = pheader->sequence;
	uint8_t fun1;
	//beacon,  resp , compound,  mac control
	if (uwb_node.header.dist != uwb_node.id && uwb_node.header.dist != 0xFFFF)
		return ;  //不过如果开启了帧过滤的话就不必了
	switch (frame_type) {
	case beacon_frame:
		UWB_Beacon_Payload_t *pbeacon = (UWB_Beacon_Payload_t*) ppayload;
		if(uwb_node.state == outside){
			//同步
			current_micro_slot = 0;
			Reset_Timer();  //复位计数器
			superframe.cfp_macro_slot_num = pbeacon->CFP_num;
			superframe.anchor_id = uwb_node.header.src;
			superframe.pan_id = uwb_node.header.pan_id;
			superframe.cap_macro_slot_num = pbeacon->CAP_num;
			enqueueTask(prepare_join, uwb_node.header.src);
		}else{
			enqueueTask(uwb_handle_beacon, uwb_node.header.src);
		}
		break;
	case data_frame:
		UWB_Function1_t *pdata = (UWB_Function1_t*) ppayload;
		fun1 = pdata->function;
		switch (fun1) {
		case UWB_Ranging_Resp:
			/**
			* 停止超时定时器
			*/
			DISABLE_COMP(my_timer.htim);
			enqueueTask(uwb_handle_resp, uwb_node.header.src);
			break;
		default:
			//unsupported function
			break;
		}
		break;
	case compound_frame:
		UWB_Function1_t *pfun1 = (UWB_Function1_t*) ppayload;
//		UWB_Function2_t *pfun2;
		fun1 = pfun1->function;
		switch (fun1) {
		case UWB_Ranging_Resp:
			/**
			* 停止超时定时器
			*/
			DISABLE_COMP(my_timer.htim);
			enqueueTask(uwb_handle_resp, uwb_node.header.src);
			UWB_Function2_t * pfun2 = (UWB_Function2_t*) (ppayload + RESP_PAYLOAD_LEN);
			enqueueTask(uwb_handle_func2, uwb_node.header.src);
			break;
		default:
			//unsupported function
			break;
		}
		break;
	case ack_frame:
		if(uwb_node.state == outside && uwb_node.sub_state == applying && applying_anchor == uwb_node.header.src){
			/**
			 * @TODO positive ack
			 */
			DISABLE_COMP(my_timer.htim);
			uwb_node.state = member;
			enqueueTask(valid_anchor, uwb_node.header.src);
		}
		break;
	case mac_cmd_frame:
		//来自锚节点的MAC控制帧
		enqueueTask(uwb_handle_mac, uwb_node.header.src);
		break;
	default:
		break;
	}
}

/**
 * @TODO  在member状态接收到beacon帧
 * 1. 是当前锚节点，查看是否测距
 * 2. 非当前锚节点，记录but invalid
 */
/**
 * @TODO  big big problem
 * beacon 帧的那些东西尚且不知道该如何 ~
 */
//in  member  state
void uwb_handle_beacon(uint16_t id){

	UWB_Beacon_Payload_t *pbeacon = (UWB_Beacon_Payload_t*) (uwb_node.rxBuffer + UWB_MAC_HEADER_LEN);
	uint16_t * pTags = pbeacon->IDs;
	uint32_t next_comp = 0;
	uint32_t tick1, tick2, delta;
//	slot_alloc_node_t* panchor;
	if(ranging_anchor_values.panchor->slot_alloc.node_id == id){
		//复位
		Reset_Timer();  //复位计数器   //话说这个复位有成功吗？
		current_micro_slot = 0;
//		ranging_anchor_values.panchor->slot_alloc.absence = 0; //连续几次没有收到锚节点消息   现在不需要这个了，因为有超时接收定时器
		for(int i = 0; i< pbeacon->CFP_num ; i++){
			if(*(pTags+i) == uwb_node.id){
				tick1 = my_timer.htim->Instance->CNT;
				ranging_anchor_values.panchor->slot_alloc.miss = 0; //清除不良记录
				ranging_anchor_values.panchor->slot_alloc.macro = i+1;
				ranging_anchor_values.panchor->slot_alloc.micro1 = GET_MICRO_SLOT1(i+1);  //这个是不是要很长的时间啊？ 我看看哈
				ranging_anchor_values.panchor->slot_alloc.micro2 = GET_MICRO_SLOT2(i+1);
				ranging_anchor_values.panchor->slot_alloc.micro3 = GET_MICRO_SLOT3(i+1);
				next_comp = MICRO_TB_NUM * ranging_anchor_values.panchor->slot_alloc.micro1 - COMPARE_OFFSET;
				prepare_poll(ranging_anchor_values.panchor->slot_alloc.node_id);
				Tag_Set_Compare(next_comp, poll_ranging);
				tick2 = my_timer.htim->Instance->CNT;
				return;
			}
		}
		//不在其中 ?
		ranging_anchor_values.panchor->slot_alloc.miss ++;
		if(ranging_anchor_values.panchor->slot_alloc.miss > uwb_node.interval + 1){
			//outside了
			uwb_node.state = outside;
			uwb_node.sub_state = idle;
			ranging_anchor_values.panchor->slot_alloc.absence = 0;
			ranging_anchor_values.panchor->slot_alloc.miss = 0;
			ranging_anchor_values.panchor->slot_alloc.is_valid = 0;
		}
		UWB_ENABLE_RX(&uwb_node.device->ports[0]);
	}else{
		//只要记录anchor即可
		hear_an_anchor(id, uwb_node.header.pan_id);  //可能需要更换子网了
		UWB_ENABLE_RX(&uwb_node.device->ports[0]);
	}
}

void uwb_handle_resp(uint16_t id){

	UNUSED(id);  // 避免unused警告

	uint32 final_tx_time;
	if(uwb_node.header.sequence != uwb_node.sequence && uwb_node.sub_state != polling){
		return;
	}
	ranging_anchor_values.poll_tx_ts = get_tx_timestamp_u64(&(uwb_node.device->ports[0]));
	ranging_anchor_values.resp_rx_ts = get_rx_timestamp_u64(&(uwb_node.device->ports[0]));
	/* Compute final message transmission time. See NOTE 10 below. */
	final_tx_time = (ranging_anchor_values.resp_rx_ts + (RESP_RX_TO_FINAL_TX_DLY_UUS * UUS_TO_DWT_TIME)) >> 8;

	ranging_anchor_values.final_tx_ts = (((uint64)(final_tx_time & 0xFFFFFFFEUL)) << 8) + uwb_node.device->antDelay; //时间戳

	packFinal(uwb_node.txBuffer);
	//send
	UWB_SEND_RANGING_FINAL(uwb_node.txBuffer, final_tx_time);
	uwb_node.sub_state = finaling;
	//根本不需要再关心后续的内容了，没有了 ~
	//进入“休眠” idle，等待下一次的定位
	Tag_Set_GotoSleep(uwb_node.interval * SUPERFRAME_TB_NUM - 5);


}


//释放，更改 ~ 不常用
void uwb_handle_mac(uint16_t id){

}

/**
 * 1. 使能接收
 * 2. 复位missing
 */
void tag_wakeup_radio(void){

	UWB_ENABLE_RX(&uwb_node.device->ports[0]);
	uwb_node.sub_state = listening;
	ranging_anchor_values.panchor->slot_alloc.miss = 0;  //但是没有哪里还设置了什么啊？
	//重置时间
	Reset_Timer();
}

/**
 * 1. 计算信号强度
 * 2. 记录有效性，准备定位通信
 * 3. 记录ranging_value
 * valid_anchor(uwb_node.header.src);
 */
void valid_anchor(uint16_t id){

	//肯定应答
	slot_alloc_node_t* panchor = find_anchor_in_table(id);
	panchor->slot_alloc.signal = uwb_calculate_power(&uwb_node.device->ports[0]);  //这个结果为什么是0 ？
	panchor->slot_alloc.is_valid = 1;
	panchor->slot_alloc.absence = 0;
	panchor->slot_alloc.miss = 0;

	uwb_node.sub_state = listening;
	ranging_anchor_values.panchor = panchor;
	//Tag_Set_Compare(SUPERFRAME_TB_NUM + MICRO_TB_NUM - COMPARE_OFFSET, beacon_timeout_cb);
	//开启接收
	UWB_ENABLE_RX(&uwb_node.device->ports[0]);
	//设置beacon监听超时
	Tag_Start_Monitor();

}

//暂时是不需要了，使用TIM2的CCR4代替
//void beacon_timeout_cb(void){
//
//	ranging_anchor_values.panchor->slot_alloc.absence ++;
//	if(ranging_anchor_values.panchor->slot_alloc.absence > 5){
//		//移除
//		remove_anchor_from_table(ranging_anchor_values.panchor->slot_alloc.node_id);
//		//状态切换
//		uwb_node.state = outside;
//		uwb_node.sub_state = idle;
//		//开启接收
//		UWB_ENABLE_RX(&uwb_node.device->ports[0]);
//	}else{
//		/**
//		 * @TODO 也许可以设置一下休眠之类的，其实这个时候应该还是处于接收状态  设置了帧过滤其实还好的
//		 */
//		Tag_Set_Waiting( SUPERFRAME_TB_NUM , resp_timeout_cb);
//	}
//
//}


/**
* @TODO   done
* 1. new an anchor item
* 2. pack and write join_reqeust_msg  //FIFO
*/
void prepare_join(uint16_t id){

	//根据自己的ID选择一个ALOHA时隙发送请求
	uint8_t random_slot = uwb_node.id % superframe.cap_macro_slot_num;
	//为什么好像是要花很长时间
	uint8_t groups = superframe.cfp_macro_slot_num/3 +((superframe.cfp_macro_slot_num%3 !=0)?1:0);
	uint32_t next_compare = MICRO_TB_NUM * (1 + groups * 9 + random_slot *2) - COMPARE_OFFSET;  //2858
	applying_anchor = id;
	hear_an_anchor(id, superframe.pan_id);
	UWB_Mac_Frame_t* joing_msg = (UWB_Mac_Frame_t*)uwb_node.txBuffer;
	joing_msg->header.control = MAC_CMD_CONTROL;
	joing_msg->header.sequence = ++ uwb_node.sequence ;
	joing_msg->header.pan_id = uwb_node.header.pan_id;
	joing_msg->header.dist = id;
	joing_msg->header.src = uwb_node.id;
	joing_msg->function = UWB_Cmd_Req;
	joing_msg->interval = uwb_node.interval;
	uwb_node.sub_state = ready_apply;
	UWB_Write_Tx_Buffer(uwb_node.txBuffer, JOIN_REQ_MSG_LEN);
	Tag_Set_Compare(next_compare, join_request);

}

//callback 1
void join_request(void){
	uwb_node.sub_state = applying;
	Tag_Set_Waiting(2 * MICRO_TB_NUM, join_timeout_cb);
	UWB_StartTx(1); //expect
}

/**
 *
 */
void prepare_poll(uint16_t id){

	packPoll(uwb_node.txBuffer);
	UWB_Write_Tx_Buffer(uwb_node.txBuffer, POLL_MSG_LEN);
	timer_tick = my_timer.htim->Instance->CNT;
	uwb_node.sub_state = ready_ranging;

}

/**
 * @TODO
 * 准备好poll数据包
 */
//callback to start poll tx
void poll_ranging(void){

	UWB_StartTx(1); //expect
	uwb_node.sub_state = polling;   //是不是都很难成功到此处？
	Tag_Set_Waiting(4 * MICRO_TB_NUM, resp_timeout_cb);

}

//callback 3
//锚节点没有立马响应？，有的啊，这不是很快就响应了的吗？
void join_timeout_cb(void){
	//等待下一个beacon帧
	uwb_node.sub_state = idle;
	/**
	 * @TODO  确认一下是否处于接收？
	 * 应当继续处于接收状态的，因为设置了要求是发送了join request之后expect
	 */

}

//callback 4
void resp_timeout_cb(void){
	//shouldn't but maybe   why is it?
	//go to sleep
	UWB_DISABLE_RX(&uwb_node.device->ports[0]);  //等待下一次唤醒
	uwb_node.sub_state = sleeping;  //处于睡眠状态 另一个定时器已经设置了

}


void Tag_lose_anchor(){
	//移除
	remove_anchor_from_table(ranging_anchor_values.panchor->slot_alloc.node_id);
	//状态切换
	uwb_node.state = outside;
	uwb_node.sub_state = idle;
	//开启接收
	UWB_ENABLE_RX(&uwb_node.device->ports[0]);
}



/**
 * @TODO function2数据  这个时候这个东西可是变化了的
 */
static void packPoll(uint8_t* pbuffer){


	UWB_Data_Frame_t* poll_msg = (UWB_Data_Frame_t*)pbuffer;
	poll_msg->header.control = POLL_CONTROL;
	poll_msg->header.sequence = uwb_node.sequence;
	poll_msg->header.pan_id = ranging_anchor_values.panchor->slot_alloc.pan_id;
	poll_msg->header.dist = ranging_anchor_values.panchor->slot_alloc.node_id;
	poll_msg->header.src = uwb_node.id;
	poll_msg->payload1.function = UWB_Ranging_Poll;

}

static void packFinal(uint8* pbuffer){

	UWB_Data_Frame_t* final_msg = (UWB_Data_Frame_t*)pbuffer;
	final_msg->header.control = FINAL_CONTROL;
	final_msg->header.sequence = uwb_node.sequence;   //自己
	final_msg->header.pan_id = ranging_anchor_values.panchor->slot_alloc.pan_id;
	final_msg->header.dist = ranging_anchor_values.panchor->slot_alloc.pan_id;
	final_msg->header.src = uwb_node.id;
	final_msg->payload1.function = UWB_Ranging_Final;
	final_msg->payload1.final_payload.poll_tx_ts = ranging_anchor_values.poll_tx_ts;
	final_msg->payload1.final_payload.resp_rx_ts = ranging_anchor_values.resp_rx_ts;
	final_msg->payload1.final_payload.final_tx_ts = ranging_anchor_values.final_tx_ts;

}

#endif


