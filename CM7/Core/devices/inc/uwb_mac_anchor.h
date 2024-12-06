/*
 * uwb_mac_anchor.h
 *
 *  Created on: Sep 28, 2024
 *      Author: 24848
 */

#ifndef DEVICES_INC_UWB_MAC_ANCHOR_H_
#define DEVICES_INC_UWB_MAC_ANCHOR_H_

#include "main.h"
#include "uwb_mac.h"

//一轮周期大约17s   //		1099511627775
//一秒钟走过的tick
//延迟发送的设定时间是8ns精度的 	低9位全0


typedef struct{

	uint8_t tx_b_valid;
	uint8_t resp1;
	uint8_t resp2;
	uint8_t resp3;

	uint64_t Tx_Beacon;
	uint64_t Next_Beacon;

	uint64_t Tx_resp1;
	uint64_t Tx_resp2;
	uint64_t Tx_resp3;

}Times_Struct_t;


void initAnchor(void);

void start_prepare_beacon(void);
void Anchor_Inc_Group(void);

void Anchor_Resp_Req(uint16_t tag_id, uint8_t tag_seq, uint8_t tag_interval);

void prepare_beacon(uint16_t id);
void send_beacon(void);

void poll_timeout_cb(uint8_t _index);

void resp_issue_cb(uint8_t _index);
void myTxDoneCb(void);
void beaconDoneCb(void);

void final_timeout_cb(uint8_t _index);

void calculate_distance(uint16_t index);

void anchor_parse_ranging(uint16_t microSlot);

//若是先不考虑这个测角？  或是交给另外一个内核？
void anchor_parse_pdoa(uint8_t pdoa_id);

void Upload_Data(volatile UWB_RangingValue_t* pValues, uint8_t num);

#endif /* DEVICES_INC_UWB_MAC_ANCHOR_H_ */
