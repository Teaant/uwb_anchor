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

typedef enum{
	anchor =1,
	tag =2,
}UWB_Role_t;
//我大概知道了这个enum其实就是说是一个数字常量，如我们所见到的那些一样
#define ANCHOR	1
#define TAG		2

#endif /* DEVICES_INC_UWB_RANGING_H_ */
