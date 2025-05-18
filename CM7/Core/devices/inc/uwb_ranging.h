/*
 * uwb_ranging.h
 *
 *  Created on: Sep 6, 2024
 *      Author: 24848
 */

#ifndef DEVICES_INC_UWB_RANGING_H_
#define DEVICES_INC_UWB_RANGING_H_

typedef enum{
	no_delay = 0,
	need_delay = 1,
}If_Delay_t;

typedef enum{
	no_expect = 0,
	expect = 1,
}If_Expected_t;

#define ANCHOR	1
#define TAG		2

#endif /* DEVICES_INC_UWB_RANGING_H_ */
