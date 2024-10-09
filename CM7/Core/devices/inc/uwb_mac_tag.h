/*
 * uwb_mac_tag.h
 *
 *  Created on: Sep 28, 2024
 *      Author: 24848
 */

#ifndef DEVICES_INC_UWB_MAC_TAG_H_
#define DEVICES_INC_UWB_MAC_TAG_H_

#include "main.h"

void initTag(void);

void tag_parse_ranging(uint16_t now_slot);

void tag_wakeup_radio(void);

/**
 * @TODO 函数参数包括 是否是复制传递
 */
void uwb_handle_func2(uint16_t id);


void uwb_handle_beacon(uint16_t id);
void uwb_handle_resp(uint16_t id);
void uwb_handle_mac(uint16_t id);


// outside
void prepare_join(uint16_t id);
void join_request(void);
void valid_anchor(uint16_t id);

void prepare_poll(uint16_t id);
void poll_ranging(void);

void beacon_timeout_cb(void);
void join_timeout_cb(void);
void resp_timeout_cb(void);     // shouldn't but  it happened


void Tag_lose_anchor(void);

#endif /* DEVICES_INC_UWB_MAC_TAG_H_ */
