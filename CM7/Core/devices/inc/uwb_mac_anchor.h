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
