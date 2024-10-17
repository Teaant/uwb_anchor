/*
 * uwb_mac_anchor.c
 *
 *  Created on: Sep 28, 2024
 *      Author: 24848
 */


#include "uwb_mac_anchor.h"
#include "uwb_mac.h"

#include "mem_manager.h"
#include "task_manager.h"

#include "corecomm.h"
#include "agent.h"


#if(RANGING_ROLE == ANCHOR)
//所有标签的结果上报给应用层，当前只考虑连续三个标签的情况

extern UWB_Node_t uwb_node;
extern Timer_t my_timer;

volatile static uint32_t timer_tick;
//typedef void(*poll_timeout_cb)(uint8_t index);
//typedef void(*resp_issue_cb)(uint8_t index);
//typedef void(*final_timeout_cb)(uint8_t index);
//
//poll_timeout_cb  poll_timeout_cbs[3] = {poll_timeout_cb1, poll_timeout_cb2 , poll_timeout_cb3};
//resp_issue_cb  	resp_issue_cbs[3] = {resp_issue_cb1, resp_issue_cb2 , resp_issue_cb3};
//final_timeout_cb	final_timeout_cbs[3] = {final_timeout_cbs1, final_timeout_cbs2, final_timeout_cbs3};
//

volatile static uint8_t tags_num = 0;
slot_alloc_node_t* tags_table, *ptail;//还是加一个尾巴吧，不好看总是

volatile static uint8_t ranging_group_num = 0;

volatile  uint8_t ranging_num;
volatile  uint8_t ranging_groups = 0;
volatile UWB_RangingValue_t  ranging_tags_value[51] __attribute__((section(".shared")));

volatile UWB_Data_Frame_t resp_buffer ={0,};

volatile UWB_Msg_Header_t  req_ack_buffer = {0, };   //ACK帧，没有内容！

static uint8_t start_cap_microSlot = 1;  //CAP起始微时隙号
static uint16_t current_micro_slot = 0;

//static uint8_t timeout_tag_seq;
volatile  int8_t waiting_index  = -1;   //hua

volatile uint16_t pdoa_rx = 0;

extern volatile input_reg_params_t   input_data __attribute__((section(".shared")));

/**
 * 一次最多51个节点同时测距  = 3 * 17
 */

/**
 * @TODO  设置每一组的开始的回调
 */

void initAnchor(void){

	tags_table = NULL;
	ptail = NULL;

	//初始化resp的数据缓冲区，后续只需要修改 dist， sequence即可
	resp_buffer.header.control = RESP_CONTROL;
	resp_buffer.header.dist = 0xFFFF;
	resp_buffer.header.src = uwb_node.id;
	resp_buffer.header.pan_id = uwb_node.pan_id;
	resp_buffer.header.sequence = 0;
	resp_buffer.payload1.function = UWB_Ranging_Resp;

	//初始化req ack的buffer，这样应该也许可以节省不少的时间的呢？
	req_ack_buffer.control = ACK_FRAME_CONTROL;
	req_ack_buffer.dist = 0xFFFF;
	req_ack_buffer.src = uwb_node.id;
	req_ack_buffer.pan_id = uwb_node.pan_id;
	req_ack_buffer.sequence = 0;

	//先初始化一下哈，就是那边确实不是等于0，可能是那个链接文件没给初始化的，是这样的
	for(int i =0; i< 51; i++){
		memset((uint8_t*)(ranging_tags_value+i), 0, sizeof(UWB_RangingValue_t));
	}
	//使能PDOA
	UWB_ENABLE_RX(&uwb_node.device->ports[1]);
	UWB_ENABLE_RX(&uwb_node.device->ports[2]);
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

static void add_new_tag(uint16_t id, uint8_t interval){

	slot_alloc_node_t* ptag = find_tag_in_table(id);
	//还是这般？
	if(ptag){
		//没有设置pan_id啊，本身就也是不太需要的
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
	slot_alloc_node_t *p, *pre, *q;
	p = tags_table;
	pre = NULL;
	while(p != NULL){
		if(p->slot_alloc.absence > p->slot_alloc.interval + 1){
			q = p;
			if(p == tags_table){
				tags_table = p->pnext;
			}else{
				pre->pnext = p->pnext;
			}
			p = p->pnext;
			memp_free(q);
			tags_num --;
		}else{
			pre = p;
			p = p->pnext;
		}
	}
}

/**
 * 1. pack beacon数据包 并且写入DW1000芯片
 * 2. 设置beacon发送定时器
 */
//标签节点会交换着下线
void prepare_beacon(uint16_t id){

	UNUSED(id);  //避免警告
	/**
	 * @Tip  关闭UWB接收机
	 */
//	Anchor_Stop_CompareTag(1);
//	Anchor_Stop_CompareTag(2);
//	Anchor_Stop_CompareTag(3);

	UWB_DISABLE_RX(&uwb_node.device->ports[0]);
	/**
	 * @TODO 上报数据 ~
	 */
	//串口也行，反正标签是静止的
	Upload_Data(ranging_tags_value, ranging_num);

	UWB_Beacon_Frame_t* pbeacon = (UWB_Beacon_Frame_t*)uwb_node.txBuffer;

	pbeacon->header.control = BEACON_CONTROL;
	pbeacon->header.dist = 0xFFFF;   //广播地址
	pbeacon->header.pan_id = uwb_node.pan_id;
	pbeacon->header.sequence = ++ uwb_node.sequence;
	pbeacon->header.src = uwb_node.id;
	pbeacon->payload.period = MICRO_TB_NUM;
	ranging_num = 0;   //为什么这个不变的呢？怎么是在变化的吗？
	ranging_groups = 0;
	ranging_group_num = 0;

	//先更新一下标签表
	refresh_table();
	volatile slot_alloc_node_t* p = tags_table;

	while (p != NULL) {
		p->slot_alloc.time_to_locate --;
		if (p->slot_alloc.time_to_locate == 0) {
			if (ranging_num == 51) {
				p->slot_alloc.time_to_locate = 1; //留到下一轮，但是也许可以把他们往前移，这样可以保证下一次有机会，假设前面已经很多的话
			} else {
				p->slot_alloc.time_to_locate = p->slot_alloc.interval; //回到初始数值，你看，总是有忽略的地方
				ranging_tags_value[ranging_num].ptag = p;
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

	pbeacon->payload.CFP_num = ranging_num;
	ranging_groups = ranging_num/3 + (((ranging_num%3)!=0)?1:0);

	pbeacon->payload.CAP_num = (250 -1 - ranging_groups * 9)/2;

	start_cap_microSlot = 1 + ranging_groups * 9 ;
	//关闭接收
	UWB_Write_Tx_Buffer( uwb_node.txBuffer, BEACON_COM_LEN + ranging_num * 2);
	Anchor_Set_Compare(SUPERFRAME_TB_NUM -1, send_beacon);

}

void send_beacon(void){

	//void prepare_beacon(uint16_t id)   时候已经关闭接收了 ~
	UWB_StartTx(1); //要马上进入接收所以，此处expect_rx
	//复位计数器

//	Reset_Timer();   //在发送完成之后再重置定时器

	setTxDoneCallback(beaconDoneCb);
}

void Anchor_Inc_Group(void){

	//对应Invalid  就为了干这件事情 ~ 我还特地的设置了这么一个回调 ~

	for(int i =0 ; i< 3; i++){
		ranging_tags_value[(ranging_group_num - 1)*3 + i].is_valid = 0; //invalid，之后是不会有的
	}

	ranging_group_num ++;
	//设置节点超时
	uint8_t tag_index = (ranging_group_num-1) * 3;
	if(ranging_group_num <= ranging_groups){
		for(int i = 0; i< 3; i++){
			if(ranging_tags_value[tag_index + i].is_valid == 0){
				Anchor_Set_CompareTag(i+1, (1+ ranging_tags_value[tag_index + i].ptag->slot_alloc.micro1) * MICRO_TB_NUM, poll_timeout_cb, tag_index + i);
			}
		}
		Anchor_Set_Compare( (1 + ranging_group_num *9)* MICRO_TB_NUM, Anchor_Inc_Group); //下一组的
	}else{
		Anchor_Set_Compare(SUPERFRAME_TB_NUM/2, start_prepare_beacon);
	}
}

//还是决定把这个任务交给main函数
void start_prepare_beacon(void){
	//这个地方有一个16就真的很奇怪，假设现在是因为标签的原因？   但是为什么呢？
	enqueueTask(prepare_beacon,  uwb_node.id);
}


//若是未曾如约定收到标签的内容  //实际上我应该是这样子的
void poll_timeout_cb(uint8_t _index){

	//记录absence
	ranging_tags_value[_index].ptag->slot_alloc.absence ++;
	//本次测距不能正常完成 ！
	ranging_tags_value[_index].is_valid = 0;
	//Tanya
	Anchor_Stop_CompareTag((_index%3)+1);  //可算是没有继续了

}

//那其实这个好像也并不需要很多了 ！
void resp_issue_cb(uint8_t _index){

//	Anchor_Stop_CompareTag(_index%3 + 1);  //那就手动关闭吧都

	resp_buffer.header.dist = ranging_tags_value[_index].ptag->slot_alloc.node_id;
	resp_buffer.header.sequence = ranging_tags_value[_index].sequence;
	//关闭接收
	UWB_DISABLE_RX(&uwb_node.device->ports[0]);
	UWB_Write_Tx_Buffer((uint8_t*)&resp_buffer, RESP_MSG_LEN);
	UWB_StartTx(1);    //to-be-continue
	//设置final消息的超时接收  Tanya_delete   no need
	//resp_tx
	//这边还没有读取完然后就读取了接收时间戳，所以导致后面的计算有问题
	waiting_index = _index;
#if(Tanya_Test)
	uint32_t timer_tick = my_timer.htim->Instance->CNT;    //81，写进去也才1个100us，怎么发送完就要这么多的呢？
#endif
	setTxDoneCallback(myTxDoneCb);
	//关闭定时器，

	//	Anchor_Set_CompareTag(1, (1 + ranging_tags_value[_index].ptag->slot_alloc.micro3)*MICRO_TB_NUM, final_timeout_cb1, _index);

}

void myTxDoneCb(void){

	if(waiting_index >= 0){
		ranging_tags_value[waiting_index].resp_tx_ts = get_tx_timestamp_u64(&uwb_node.device->ports[0]);
#if(Tanya_Test)
		uint32_t timer_tick = my_timer.htim->Instance->CNT;   //93！怎么会需要这么长的时间吗？
#endif
		waiting_index = -1;
	}
}

//Reset_Timer有点奇怪的 ~ 所以是怎么回事？ 怎么这边Reset的时候会怎么样吗？
void beaconDoneCb(void){
	Reset_Timer();
	current_micro_slot = 0;
	if(ranging_groups == 0){
		Anchor_Set_Compare(SUPERFRAME_TB_NUM/2, start_prepare_beacon);  //难不成这个比较还是必须是相等的时候才行吗？   //先只发一次看看 ~
		return;
	}else{
		ranging_group_num = 1; //第一组
		for(int i =0; i < 3; i++){
			if(ranging_tags_value[(ranging_group_num-1) *3 + i].is_valid == 1){
				Anchor_Set_CompareTag(i+1, (1+ ranging_tags_value[(ranging_group_num-1) *3 + i].ptag->slot_alloc.micro1) * MICRO_TB_NUM, poll_timeout_cb, (ranging_group_num-1)*3 + i);
			}
		}
		Anchor_Set_Compare( (1 + ranging_group_num *9)* MICRO_TB_NUM, Anchor_Inc_Group);   //设置下一组的开始
	}
}



//ranging_group_num
//其实好像没必要了，之后能不能接收到都没什么关系， 而且可能也不必有特别严格的时间同步

void final_timeout_cb1(uint8_t _index){

//	ranging_tags_value[(ranging_group_num-1)*3 + 0].is_valid = 0;

}

void anchor_parse_ranging(uint16_t microSlot){

	uint8_t isCFP = 0;
	uint8_t macroSlot = 0 ;

	if(microSlot < start_cap_microSlot && microSlot > 0){
		isCFP = 1;
		macroSlot = (microSlot - 1)/9 *3 + (microSlot - 1)%3 + 1;   //获得当前宏时隙号，对应着是节点，起始索引是1
	}
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
	case data_frame:
		UWB_Function1_t *pdata = (UWB_Function1_t*) ppayload;
		fun1 = pdata->function;
		switch (fun1) {
		case UWB_Ranging_Poll:
			if(!isCFP) {
				UWB_ENABLE_RX(&uwb_node.device->ports[0]);
				break;
			};
			//确定是来自对应的节点的
#if(Tanya_Test)
			uint32_t timer_tick = my_timer.htim->Instance->CNT;
#endif
			if((ranging_tags_value[macroSlot -1].ptag->slot_alloc.node_id != uwb_node.header.src) || (ranging_tags_value[macroSlot-1].is_valid != 1)) break;
			Anchor_Stop_CompareTag((macroSlot -1)%3 +1);
			//这不应该是先出现的吗？
			ranging_tags_value[macroSlot -1].poll_rx_ts = get_rx_timestamp_u64(&uwb_node.device->ports[0]);
			ranging_tags_value[macroSlot -1].sequence = uwb_node.header.sequence;
			ranging_tags_value[macroSlot -1].ptag->slot_alloc.absence = 0;
			//设置Resp的发送
			Anchor_Set_CompareTag((macroSlot -1)%3 + 1, (ranging_tags_value[macroSlot-1].ptag->slot_alloc.micro2) * MICRO_TB_NUM ,resp_issue_cb, macroSlot -1);
			UWB_ENABLE_RX(&uwb_node.device->ports[0]);

			break;
		case UWB_Ranging_Final:
			if(!isCFP) {
				UWB_ENABLE_RX(&uwb_node.device->ports[0]);
				break;
			};
			//源地址对应
#if(Tanya_Test)
			timer_tick = my_timer.htim->Instance->CNT;   //162？
#endif
			if(ranging_tags_value[macroSlot-1].ptag->slot_alloc.node_id != uwb_node.header.src ||ranging_tags_value[macroSlot-1].is_valid != 1) break;
			ranging_tags_value[macroSlot -1].final_rx_ts = get_rx_timestamp_u64(&uwb_node.device->ports[0]);
			ranging_tags_value[macroSlot -1].poll_tx_ts = pdata->final_payload.poll_tx_ts;
			ranging_tags_value[macroSlot -1].resp_rx_ts = pdata->final_payload.resp_rx_ts;
			ranging_tags_value[macroSlot -1].final_tx_ts = pdata->final_payload.final_tx_ts;
			ranging_tags_value[macroSlot -1].is_available = 1;
			ranging_tags_value[macroSlot -1].ptag->slot_alloc.absence = 0;
			enqueueTask(calculate_distance, macroSlot -1); //对应着索引
			UWB_ENABLE_RX(&uwb_node.device->ports[0]);
			break;
		case UWB_App_Data:
			//unable to implement
//			break;
		default:
			UWB_ENABLE_RX(&uwb_node.device->ports[0]);
			break;
		}
		break;
	//申请加入网络
	case mac_cmd_frame:
#if(Tanya_Test)
		uint32_t timer_tick = my_timer.htim->Instance->CNT;   // 2880  2880
#endif
		UWB_Mac_Frame_t* pmac = (UWB_Mac_Frame_t*) uwb_node.rxBuffer;
		switch(pmac->function){
		case UWB_Cmd_Req:
			if(isCFP) {
				UWB_ENABLE_RX(&uwb_node.device->ports[0]);
				return;
			}    //not the appropriate time to do this ~
			if(tags_num < 64){  //这是由可分配的内存空间决定的
				Anchor_Resp_Req(uwb_node.header.src, uwb_node.header.sequence, pmac->interval);   //？
			}else{
				UWB_ENABLE_RX(&uwb_node.device->ports[0]);
			}
			break;
		case UWB_Cmd_Modify:
			//unable to implement now
			break;
		case UWB_Cmd_Release:    //是不是可以标签主动释放？
			//unable to implement now
			break;
		default:
			break;
		}
		break;
	case compound_frame:
		//unable to implement
		break;
	default:
		break;
	}
}

void Anchor_Resp_Req(uint16_t tag_id, uint8_t tag_seq, uint8_t tag_interval){

#if(Tanya_Test)
	uint32_t timer_tick = my_timer.htim->Instance->CNT;
#endif
	req_ack_buffer.dist = tag_id;
	req_ack_buffer.sequence = tag_seq;
	UWB_Write_Tx_Buffer((uint8_t*)&req_ack_buffer, JOIN_RESPONSE_LEN);
	UWB_StartTx(1);
	add_new_tag(tag_id, tag_interval);  //main

}

void calculate_distance(uint16_t index){

	int64  tof_dtu;

	ranging_tags_value[index].R1 = getDeltaT(ranging_tags_value[index].resp_rx_ts , ranging_tags_value[index].poll_tx_ts);
	ranging_tags_value[index].R2 = getDeltaT(ranging_tags_value[index].final_rx_ts , ranging_tags_value[index].resp_tx_ts);
	ranging_tags_value[index].D1 = getDeltaT(ranging_tags_value[index].resp_tx_ts , ranging_tags_value[index].poll_rx_ts);
	ranging_tags_value[index].D2 = getDeltaT(ranging_tags_value[index].final_tx_ts , ranging_tags_value[index].resp_rx_ts);

	//来自文档(App Note APS013)的神秘公式   神秘公式不神秘 ~
	tof_dtu = (int64)(((double)ranging_tags_value[index].R1 * (double)ranging_tags_value[index].R2 - (double)ranging_tags_value[index].D1 * (double)ranging_tags_value[index].D2) / ((double)ranging_tags_value[index].R1 + (double)ranging_tags_value[index].R2 + (double)ranging_tags_value[index].D1 + (double)ranging_tags_value[index].D2));
	ranging_tags_value[index].distance = (float)tof_dtu*DWT_TIME_UNITS*SPEED_OF_LIGHT;
	ranging_tags_value[index].ptag->slot_alloc.times ++;
	//Tanya_add  这个的单位是m ？
	if(ranging_tags_value[index].distance < 0.3)
	{
		ranging_tags_value[index].distance = 0.3;
	}
}

/**
 * @TODO  unable to implement
 */
void anchor_parse_pdoa(uint8_t pdoa_id){
	pdoa_rx++;
}

//到时候交给另外的去实现也是可以的  清零
//__weak void Upload_Data(volatile UWB_RangingValue_t* pValues, uint8_t num){
//	for(int i = 0; i < num ; i++){
//		memset((uint8_t*)(pValues+i), 0, sizeof(UWB_RangingValue_t));
//	}
//}

__weak void Upload_Data(volatile UWB_RangingValue_t* pValues, uint8_t num){

	if(num <= 0) return;
	uint8_t vailable_num = 0;
	input_data.my_id = uwb_node.id;
	input_data.my_pan_id = uwb_node.pan_id;

	for(int i = 0; i < num ; i++){
		if((pValues+i)->is_available == 1){
			input_data.nodes_value1[vailable_num].tag_id = (pValues+i)->ptag->slot_alloc.node_id;
			input_data.nodes_value1[vailable_num].absence  = (pValues+i)->ptag->slot_alloc.absence;
			input_data.nodes_value1[vailable_num].interval = (pValues+i)->ptag->slot_alloc.interval;
			input_data.nodes_value1[vailable_num].times = (pValues+i)->ptag->slot_alloc.times;
			input_data.nodes_value1[vailable_num].distance = (uint16_t)((pValues+i)->distance * 100); //in cm
			input_data.nodes_value1[vailable_num].angle = 0;
			vailable_num ++;
		}
		memset((uint8_t*)(pValues+i), 0, sizeof(UWB_RangingValue_t));
	}
	input_data.available = 1;
	input_data.node_num = vailable_num;
	//中断
	actCM4Interrupt(RANGING_DATA_724_3);
	genInterrupt(RANGING_DATA_724_3);



}

#endif
