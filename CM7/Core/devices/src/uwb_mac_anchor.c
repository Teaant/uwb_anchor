/*
 * uwb_mac_anchor.c
 *
 *  Created on: Sep 28, 2024
 *      Author: 24848
 */


#include <uwb_mac.h>
#include "uwb_mac_anchor.h"
#include "task_manager.h"
#include "corecomm.h"
#include "agent.h"
#include "../../../util/mem_manager.h"
#include "../../../util/msg_queue.h"

#include "tim.h"

#include <math.h>


#if(MY_ROLE == ANCHOR)

#define TEST_RESP 	0
#define DO_RESP		1


extern volatile UWB_Node_t uwb_node;

extern volatile AoADataTypeDef aoa_data[MAX_TAG];


volatile static uint8_t tags_num = 0;   //实施记录当前有多少的节点 ~
volatile slot_alloc_node_t* tags_table = NULL;

volatile static uint8_t is_ranging = 0;   //是否处于ranging阶段 ~
volatile static uint8_t ranging_group_num = 0;

extern volatile uint8_t ranging_num;
volatile  uint8_t ranging_groups = 0;

volatile UWB_RangingValue_t  ranging_tags_value[MAX_TAG];



static slot_alloc_node_t* find_tag_in_table(uint16_t id);
static void add_new_tag(uint16_t id, uint8_t interval);
static void refresh_table(void);

//还是不行，假如我处于接收模式，根本不可能实现发送的 ~
//难以实现啊，这边，要不然还是一整个呗？ 就是广播Resp消息 ~ 这样无论如何也可以，而且，还能及时进行反馈了 ~
//但是广播就广播，也没什么关系了，我只要保持，对，而且那些标签如果出于延迟发送，就是不会接收的，可以 ~
volatile uint8_t resp_times = 0;

volatile Anchor_Struct_t anchor_struct = {
#if(ENABLE_SYNC)
		.ref_id = MY_ID,
		.level = 0,
		.master_slot = 0,
		.my_slot = 0,
		.Slots = {MY_ID, 0xFFFF, 0xFFFF, 0xFFFF},
#else
		0,
#endif
};

volatile UWB_Date_Resp_t resp_buffer = {0, };

void initAnchor(void){

	tags_table = NULL;

	anchor_struct.req_ack_buffer.header.control = ACK_FRAME_CONTROL;
	anchor_struct.req_ack_buffer.header.pan_id = PAN_ID;
	anchor_struct.req_ack_buffer.header.src = MY_ID;

	resp_buffer.header.control = RESP_CONTROL;
	resp_buffer.header.dist = 0xFFFF;
	resp_buffer.header.sequence = 23;
	resp_buffer.header.pan_id = PAN_ID;
	resp_buffer.header.src = MY_ID;
	resp_buffer.function = UWB_Ranging_Resp;

	uwb_node.panchor_struct = &anchor_struct;

	for(int i = 0; i < MAX_TAG; i++){
		memset((uint8_t*)(ranging_tags_value+i), 0, sizeof(UWB_RangingValue_t));
	}

	init_mem_pool();

	//register txDone Callback
	register_tx_cb(beaconing, beacon_txdone_cb);
	register_tx_cb(ranging, resp_txdone_cb);

	ranging_num = 0;


}

void Anchor_start_run(void){
	prepare_beacon(0);
}



void anchor_parse_ranging(uint16_t microSlot){

	UWB_Rcv_Msg_t* pmsg = get_msg_from_queue();
	UWB_Msg_Header_t *pheader = (UWB_Msg_Header_t*) pmsg->rcv_data;
	uint8_t frame_type = GET_FRAMETYPE(pheader->control);
	//beacon,  resp , compound,  mac control
	uint16_t src_id = pheader->src;
	uint8_t sequence = pheader->sequence;

	if (pheader->dist != MY_ID && pheader->dist != 0xFFFF){
		//计算当前的时候， 如果属于resp时间段，那就那就不必打开接收 ~
		release_msg_buffer(pmsg);
		return;
	}
	switch(frame_type){
	case beacon_frame:
		//暂时先这样了，没有别的~
#if(!USE_TIMER)
		enable_rx_with_timeout(0);
#endif
#if(ENABLE_SYNC)

#endif
		break;
	case data_frame:

		UWB_Data_Final_t* ppayload = (UWB_Data_Final_t*)(pmsg->rcv_data);
		switch(ppayload->function){
		case UWB_Ranging_Poll:
			if(uwb_node.state != ranging) break;
			for(int i = 3 * (ranging_group_num -1); i < 3 * ranging_group_num; i++){
				if(ranging_tags_value[i].pnode->slot_alloc.node_id == src_id){
#if(!USE_TIMER)
					enable_rx_with_timeout(RX_TIMEOUT_4_2MS);
#endif
					ranging_tags_value[i].pnode->slot_alloc.absence = 0;
//					ranging_tags_value[i].sequence = sequence;
					ranging_tags_value[i].poll_rx_ts = pmsg->rx_ts;
					ranging_tags_value[i].is_valid = 1;
					release_msg_buffer(pmsg);
					return;
				}
			}
			break;
		case UWB_Ranging_Final:
#if(!USE_TIMER)
			enable_rx_with_timeout(0);
#endif
			for (int i = 3 * (ranging_group_num-2); i < 3 * (ranging_group_num -1); i++) {
				if (ranging_tags_value[i].pnode->slot_alloc.node_id == src_id && ranging_tags_value[i].is_valid == 1) {
					ranging_tags_value[i].pnode->slot_alloc.absence = 0;
					ranging_tags_value[i].final_rx_ts = pmsg->rx_ts;
					ranging_tags_value[i].poll_tx_ts = ppayload->final_payload.poll_tx_ts;
					ranging_tags_value[i].resp_rx_ts = ppayload->final_payload.resp_rx_ts;
					ranging_tags_value[i].final_tx_ts = ppayload->final_payload.final_tx_ts;
					ranging_tags_value[i].is_available = 1;
					calculate_distance(i);
					break;
				}
			}
			break;
		default:
			//计算当前的时候， 如果属于resp时间段，那就那就不必打开接收 ~
			break;
		}
		break;
	case mac_cmd_frame:
		if(uwb_node.state == ranging){
			//计算当前时候，是否是resp时间段 ~
			release_msg_buffer(pmsg);
			return;   //CAP
		}
		UWB_Mac_Payload_t * pmac = (UWB_Mac_Payload_t*)(pmsg->rcv_data + sizeof(UWB_Msg_Header_t));
		uint8_t interval = pmac->interval;
		switch(pmac->function){
		case UWB_Cmd_Req:
			if(tags_num < MAX_TAG){
				//可以给予正向，但是还需要check一下时间是否是对的哈
				anchor_struct.req_ack_buffer.header.dist = src_id;
				anchor_struct.req_ack_buffer.header.sequence = sequence;
				anchor_struct.req_ack_buffer.payload.function = UWB_Cmd_Req;
				anchor_struct.req_ack_buffer.payload.interval = interval;
				//晚一点呢？ 能很快的嘛？ 是不是会很快啊，我看一下吧 ~
				add_new_tag(src_id, interval);
				//还是增加2ms吧？ 感觉实在是有点快乐会不会？
				uint64_t tx_time = getSumT(pmsg->rx_ts, MS_2);
				UWB_Anchor_Ack_Req((uint8_t*)(&anchor_struct.req_ack_buffer), REQ_ACK_MSG_LEN);
			}
			break;
		case UWB_Cmd_Sync:
			if (uwb_node.state == ranging) {
				//计算当前时候，是否是resp时间段 ~
				release_msg_buffer(pmsg);
				return;//CAP
			}
#if(ENABLE_SYNC)

#endif
			break;
		default:
			break;
		}
		break;
	case ack_frame:
		if (uwb_node.state == ranging) {
			//计算当前时候，是否是resp时间段 ~
			release_msg_buffer(pmsg);
			return;//CAP
		}
		UWB_Mac_Payload_t * pack = (UWB_Mac_Payload_t*)(pmsg->rcv_data + sizeof(UWB_Msg_Header_t));
		switch (pack->function) {
		case UWB_Cmd_Sync:
			//对slot分配的结果 ~
			break;
		case UWB_Cmd_Req:

			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
	//结束需要释放
	release_msg_buffer(pmsg);
}



void beacon_txdone_cb(uint64_t tx_ts){


	if (ranging_num == 0) {
		uwb_node.state = non_ranging;
	} else {
		Notify_CM4(Enable_PDoA);
		uwb_node.state = ranging;
		ranging_group_num = 1;
#if(USE_TIMER)
#if(ENABLE_SYNC)
		if (anchor_struct.my_slot == 1 || anchor_struct.my_slot == 3) {
			ENABLE_TIMER6_ARR(TIMER6_8MS);
		} else {
			ENABLE_TIMER6_ARR(TIMER6_10MS);
		}
#else
			ENABLE_TIMER2_ARR(TIMER6_10MS);
		#endif
#endif
		//计算resp
		anchor_struct.resp_tx_time = getSumT(anchor_struct.anchor_times.Start_Beacon, MS_14);
	}

#if(ENABLE_SYNC)
	anchor_struct.anchor_times.Start_Beacon = getDeltaT(tx_ts, (MS_2 *(anchor_struct.my_slot&0x01)));   //好像是这样的了~
#else
	anchor_struct.anchor_times.Start_Beacon = tx_ts;
#endif
	anchor_struct.anchor_times.Next_Beacon = getSumT(tx_ts, S_TO_DWT_TIME);

	ENABLE_TIMER15_ARR(TIMER15_0_8S);

}


//锚节点应该是没什么问题的，现在看一下到底是不是标签的问题 ~
void resp_txdone_cb(uint64_t tx_ts){

	for(int i = (ranging_group_num-1) * 3 ; i < ranging_group_num * 3; i++){
		if(ranging_tags_value[i].is_valid) ranging_tags_value[i].resp_tx_ts = tx_ts;
	}
	ranging_group_num  += 1;

	if(ranging_group_num > ranging_groups) {
		uwb_node.state = non_ranging;
		//不再需要schedule timer的了
		//同时也，关闭PDOA
		DISABLE_TIMER6();
		Notify_CM4(Disable_PDoA);

	} else {
		//write in the tx_buffer of DW1000
		anchor_struct.resp_tx_time = getSumT(anchor_struct.resp_tx_time, MS_18);
//		enqueueTask(configure_resp_to_dw1000, 0);
		//计算下一个resp_tx_time
	}
}

void configure_resp_to_dw1000(uint16_t id){

	(void) id;
	//就只差最后的StartTx了
	int sum = 0;
	uint16_t* pID = &resp_buffer.ID1;
	for(int i = 0; i < 3; i++){
		if(ranging_tags_value[(ranging_group_num-1)*3+i].pnode && ranging_tags_value[(ranging_group_num-1)*3+i].is_valid == 1){
			*(pID+i) = ranging_tags_value[(ranging_group_num-1)*3].pnode->slot_alloc.node_id;
			sum += 1;
		}else {
			*(pID+i) = MY_ID;
			sum += 1;
		};
	}
	UWB_Configure_Resp_In_Buffer((uint8_t*)(&resp_buffer), RESP_MSG_LEN, 0, anchor_struct.resp_tx_time&0xFFFFFFFFFFFFFE00);

}

static void stest_resp(uint16_t id){

	configure_resp_to_dw1000(0);
	issue_resp();
}


void timer15_callback(void){

#if(TEST_RESP)
	stest_resp();  //还是没有问题的啊？ 到底是什么事
	ENABLE_TIMER15_ARR(TIMER15_1S);
#else
	if(uwb_node.state == initial){
		issue_beacon(0);
	}else{
		enqueueTask(prepare_beacon, 0);
	}
#endif
}

void timer6_callback(void){

	if(uwb_node.state == ranging){   	  //通知开启 ~  //通知关闭 ~
		//issue Resp
		ENABLE_TIMER6_ARR(TIMER6_18MS);
		Notify_CM4(Process_PDoA);
		enqueueTask(stest_resp, 0);
//		configure_resp_to_dw1000(0);
//		issue_resp();  //直接发送了 ~
	}
}


void calculate_distance(uint16_t index){

	int64  tof_dtu;
	ranging_tags_value[index].R1 = getDeltaT(ranging_tags_value[index].resp_rx_ts , ranging_tags_value[index].poll_tx_ts);
	ranging_tags_value[index].R2 = getDeltaT(ranging_tags_value[index].final_rx_ts , ranging_tags_value[index].resp_tx_ts);
	ranging_tags_value[index].D1 = getDeltaT(ranging_tags_value[index].resp_tx_ts , ranging_tags_value[index].poll_rx_ts);
	ranging_tags_value[index].D2 = getDeltaT(ranging_tags_value[index].final_tx_ts , ranging_tags_value[index].resp_rx_ts);

	//来自文档(App Note APS013)的神秘公式
	tof_dtu = (int64)(((double)ranging_tags_value[index].R1 * (double)ranging_tags_value[index].R2 - (double)ranging_tags_value[index].D1 * (double)ranging_tags_value[index].D2) / ((double)ranging_tags_value[index].R1 + (double)ranging_tags_value[index].R2 + (double)ranging_tags_value[index].D1 + (double)ranging_tags_value[index].D2));
	ranging_tags_value[index].distance = (float)tof_dtu*DWT_TIME_UNITS*SPEED_OF_LIGHT;
	ranging_tags_value[index].pnode->slot_alloc.times ++;
	//Tanya_add  这个的单位是m ？
	if(ranging_tags_value[index].distance < 0.3)
	{
		ranging_tags_value[index].distance = 0.3;
	}
	/**
	 * @TODO  Upload data
	 * 还有角度信息需要从CM4内核当中去获取
	 */
	Upload_Data(index);

}


__weak void Upload_Data(uint8_t index){
	//时间戳也打印一下看看，到底什么情况哎 ~
	if(aoa_data[index].avalible == 1 && isnormal(aoa_data[index].theta)){
		printf("%d,%.2f,%.2f\r\n", ranging_tags_value[index].pnode->slot_alloc.node_id, ranging_tags_value[index].distance, aoa_data[index].theta);
	}else{
		printf("%d,%.2f,N\r\n", ranging_tags_value[index].pnode->slot_alloc.node_id, ranging_tags_value[index].distance);
	}
}
/**
 * 1. pack beacon数据包 并且写入DW1000芯片
 */
void prepare_beacon(uint16_t id){

	//关闭先 ~
	UWB_DISABLE_RX((&uwb_node.device->ports[0]));

	UNUSED(id);  //避免警告

	volatile UWB_Beacon_Frame_t* pbeacon = &anchor_struct.beacon_frame;

	pbeacon->header.control = BEACON_CONTROL;
	pbeacon->header.dist = 0xFFFF;   //广播地址
	pbeacon->header.pan_id = PAN_ID;
	pbeacon->header.sequence = ++ uwb_node.sequence;
	pbeacon->header.src = MY_ID;
	pbeacon->payload.period = 2;

	for(int i = 0; i< ranging_num; i++){
		if(ranging_tags_value[i].is_available != 1){
			ranging_tags_value[i].pnode->slot_alloc.absence ++;
		}
		memset((uint8_t*)(&ranging_tags_value[i]), 0, sizeof(UWB_RangingValue_t));
		memset((uint8_t*)(&aoa_data[i]), 0, sizeof(AoADataTypeDef));
	}

#if(ENABLE_SYNC)
	pbeacon->sync_header.ref_id = uwb_node.panchor_struct->ref_id;
	pbeacon->sync_header.level = uwb_node.panchor_struct->level;
	memcpy((uint8_t*)(pbeacon->sync_header.Slots), (uint8_t*)(anchor_struct.Slots), 4 * sizeof(uint16_t));
#endif

	//先更新一下标签表
	refresh_table();

	ranging_num = 0;
	ranging_groups = 0;
	ranging_group_num = 0;

	slot_alloc_node_t* p = tags_table;

	while (p != NULL) {
		p->slot_alloc.time_to_locate --;
		if (p->slot_alloc.time_to_locate == 0) {
			if (ranging_num == MAX_TAG) {
				p->slot_alloc.time_to_locate = 1;
			} else {
				p->slot_alloc.time_to_locate = p->slot_alloc.interval;
				ranging_tags_value[ranging_num].pnode = p;
				ranging_tags_value[ranging_num].is_valid = 1; //记得结束之后应该把这个标志清除掉
				ranging_tags_value[ranging_num].is_available = 0;
				pbeacon->payload.IDs[ranging_num] = p->slot_alloc.node_id;
				aoa_data[ranging_num].src_car_id = p->slot_alloc.node_id;
				ranging_num += 1;
				p->slot_alloc.macro = ranging_num;
				p->slot_alloc.micro1 = GET_MICRO_SLOT1(p->slot_alloc.macro);
				p->slot_alloc.micro2 = GET_MICRO_SLOT2(p->slot_alloc.macro);
				p->slot_alloc.micro3 = GET_MICRO_SLOT3(p->slot_alloc.macro);
			}
		}
		p = p->pnext;
	}
	pbeacon->payload.Ntags = ranging_num;
	ranging_groups = ranging_num/3 + (((ranging_num%3)!=0)?1:0);

	pbeacon->payload.sCAP = 2 + ranging_groups * 9 ; //micro_slot_start ~

//	if(ranging_num == 0){
////		Notify_CM4(Disable_PDoA);
//	}

	UWB_Schedule_Beacon_Frame((uint8_t*)pbeacon, BEACON_COMMON_LEN + ranging_num * 2, anchor_struct.anchor_times.Next_Beacon, ranging_num, anchor_struct.my_slot);

	uwb_node.state = beaconing;

}


void issue_beacon(uint16_t id){


	UWB_DISABLE_RX((&uwb_node.device->ports[0]));

	UNUSED(id);  //避免警告

	volatile UWB_Beacon_Frame_t* pbeacon = &anchor_struct.beacon_frame;

	pbeacon->header.control = BEACON_CONTROL;
	pbeacon->header.dist = 0xFFFF;   //广播地址
	pbeacon->header.pan_id = PAN_ID;
	pbeacon->header.sequence = ++ uwb_node.sequence;
	pbeacon->header.src = MY_ID;
	pbeacon->payload.period = 2;

	for(int i = 0; i< ranging_num; i++){
		if(ranging_tags_value[i].is_available != 1){
			ranging_tags_value[i].pnode->slot_alloc.absence ++;
		}
		memset((uint8_t*)(&ranging_tags_value[i]), 0, sizeof(UWB_RangingValue_t));
	}

#if(ENABLE_SYNC)
	pbeacon->sync_header.ref_id = uwb_node.panchor_struct->ref_id;
	pbeacon->sync_header.level = uwb_node.panchor_struct->level;
	pbeacon->sync_header.bop_0 = uwb_node.panchor_struct->Slots[0];
	pbeacon->sync_header.bop_1 = uwb_node.panchor_struct->Slots[1];
	pbeacon->sync_header.bop_2 = uwb_node.panchor_struct->Slots[2];
	pbeacon->sync_header.bop_3 = uwb_node.panchor_struct->Slots[3];
#endif

	//查看Ranging_tags_value当中，哪一些没有is_available的，absence++

	//先更新一下标签表
	refresh_table();

	ranging_num = 0;
	ranging_groups = 0;
	ranging_group_num = 0;

	slot_alloc_node_t* p = tags_table;

	while (p != NULL) {
		p->slot_alloc.time_to_locate --;
		if (p->slot_alloc.time_to_locate == 0) {
			if (ranging_num == MAX_TAG) {
				p->slot_alloc.time_to_locate = 1;
			} else {
				p->slot_alloc.time_to_locate = p->slot_alloc.interval;
				ranging_tags_value[ranging_num].pnode = p;
				ranging_tags_value[ranging_num].is_valid = 1; //记得结束之后应该把这个标志清除掉
				ranging_tags_value[ranging_num].is_available = 0;
				pbeacon->payload.IDs[ranging_num] = p->slot_alloc.node_id;
				ranging_num += 1;
				p->slot_alloc.macro = ranging_num;
				p->slot_alloc.micro1 = GET_MICRO_SLOT1(p->slot_alloc.macro);
				p->slot_alloc.micro2 = GET_MICRO_SLOT2(p->slot_alloc.macro);
				p->slot_alloc.micro3 = GET_MICRO_SLOT3(p->slot_alloc.macro);
			}
		}
		p = p->pnext;
	}

	pbeacon->payload.Ntags = ranging_num;
	ranging_groups = ranging_num/3 + (((ranging_num%3)!=0)?1:0);

	pbeacon->payload.sCAP = 2 + ranging_groups * 9 ; //micro_slot_start ~

	uwb_node.state = beaconing;

	UWB_Issue_Beacon_Frame((uint8_t*)pbeacon, BEACON_COMMON_LEN + ranging_num * 2);

}


static slot_alloc_node_t* find_tag_in_table(uint16_t id){

	slot_alloc_node_t* ptag = tags_table;
	while(ptag != NULL){
		if(ptag->slot_alloc.node_id == id){
			break;
		}else{
			ptag = ptag->pnext;
		}
	}
	return ptag;
}

void test_add_node(uint16_t id, uint8_t interval){
	add_new_tag(id, interval);
}

static void add_new_tag(uint16_t id, uint8_t interval){

	slot_alloc_node_t* ptag = find_tag_in_table(id);  //总是需要照一次，没关系，也enqueueTask就好了 ~
	if(ptag){
		ptag->slot_alloc.node_id = id;
		ptag->slot_alloc.interval = interval;
		ptag->slot_alloc.absence = 0;
		ptag->slot_alloc.time_to_locate = 1; //这意味着下一个就是了
		return;
	}
	ptag = memp_alloc();
	if(ptag){
		ptag->slot_alloc.node_id = id;
		ptag->slot_alloc.absence = 0;
		ptag->slot_alloc.interval = interval;
		ptag->slot_alloc.time_to_locate = 1;
		//在链表前面插入
		ptag->pnext = tags_table;
		tags_table = ptag;
		tags_num ++;
	}
}

/**
 * 1. NULL
 * 2. 1, 0
 * 3. 1, 3
 * 4. 2， 1，3
 * 5. 2， 3，1
 */
static void refresh_table(void){
	slot_alloc_node_t *p, *q;
	p = tags_table;
//	pre = NULL;
	while(p != NULL){
		if(p->slot_alloc.absence > p->slot_alloc.interval + 1){
			q = p;
			if(p == tags_table){
				tags_table = p->pnext;
			}
//			else{
//				pre->pnext = p->pnext;
//			}
			p = p->pnext;
			memp_free(q);
			tags_num --;
		}else{
//			pre = p;
			p = p->pnext;
		}
	}
}



#endif
