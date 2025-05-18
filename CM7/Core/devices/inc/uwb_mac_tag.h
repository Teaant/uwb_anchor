/*
 * uwb_mac_tag.h
 *
 *  Created on: Sep 28, 2024
 *      Author: 24848
 */

#ifndef DEVICES_INC_UWB_MAC_TAG_H_
#define DEVICES_INC_UWB_MAC_TAG_H_

#include "main.h"

typedef void(*tag_timer_callback)(void);

typedef struct{
	//所需要的东西
	volatile uint8_t macro;
	uint8_t micro1;
	uint8_t micro2;
	uint8_t micro3;

	uint8_t cap_start;

	volatile uint16_t applying_anchor;
	volatile uint16_t applying_anchor_pan;

	uint64_t poll_tx_time;
	uint64_t final_tx_time;

	volatile uint64_t beacon_start_time;

	tag_timer_callback timer6_callback;
	tag_timer_callback timer15_callback;

	uint8_t rxBuffer[200];
	uint8_t txBuffer[40];


}Tag_Struct_t;

void initTag(void);


void tag_parse_ranging(uint64_t rx_ts);


void tag_wakeup_radio(void);


void uwb_handle_beacon(uint16_t id);
void uwb_handle_resp(uint16_t id);
void uwb_handle_mac(uint16_t id);



// outside
void prepare_join(uint16_t id);
void join_request(void);
void valid_anchor(uint16_t id);

void prepare_poll(uint16_t id);
void poll_ranging(void);
void prepare_final(uint16_t id);


void Tag_lose_anchor(void);

#endif /* DEVICES_INC_UWB_MAC_TAG_H_ */
