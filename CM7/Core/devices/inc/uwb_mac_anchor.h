/*
 * uwb_mac_anchor.h
 *
 *  Created on: Sep 28, 2024
 *      Author: 24848
 */

#ifndef DEVICES_INC_UWB_MAC_ANCHOR_H_
#define DEVICES_INC_UWB_MAC_ANCHOR_H_

#include "main.h"
#include "uwb_msg.h"

#include "agent.h"

#define USE_TIMER 	1

//一轮周期大约17s   //		1099511627775
//一秒钟走过的tick
//延迟发送的设定时间是8ns精度的 	低9位全0    512 / 499.2 /10e6 / 128



typedef struct{

	//统一设置为0的时候吧，那就等一个十几秒吧 ~
	uint64_t Start_Beacon;   //BOP 0或者2的开始时间，并不是我的Beacon发送的时间
	uint64_t Tx_Beacon;
	uint64_t Next_Start_Beacon;	 //下一个时间，和下面那个相等或者 ~
	uint64_t Next_Beacon;

}Times_Struct_t;


typedef struct{

#if(ENABLE_SYNC)
	uint8_t log_slot;
	uint16_t ref_id;
	uint8_t level;
	uint8_t my_slot; //bop 1,2,3,4  //那还是0123呗
	uint8_t master_slot;
	uint8_t neighbor_num;
	uint16_t neighbors[2];
	uint8_t neighbor_slots[2];
	uint16_t Slots[4];
#endif

	uint8_t comm_range;

	Times_Struct_t anchor_times; //一些时间点 ~

	uint64_t resp_tx_time;

	UWB_Ack_Frame_t  req_ack_buffer;

	volatile UWB_Beacon_Frame_t beacon_frame;

}Anchor_Struct_t;


void initAnchor(void);


void prepare_beacon(uint16_t id);
void issue_beacon(uint16_t id);

void anchor_parse_ranging(uint16_t microSlot);


void beacon_txdone_cb(uint64_t tx_ts);
void resp_txdone_cb(uint64_t tx_ts);
void configure_resp_to_dw1000(uint16_t id);

void timer15_callback(void);
void timer6_callback(void);

//可以在这个里边Upload_Date
void calculate_distance(uint16_t index);

void Log_Data(uint16_t id);

void Upload_Data(uint16_t index);

void test_add_node(uint16_t id, uint8_t interval);

#endif /* DEVICES_INC_UWB_MAC_ANCHOR_H_ */
