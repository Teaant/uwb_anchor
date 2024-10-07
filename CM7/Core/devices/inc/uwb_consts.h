/*
 * uwb_consts.h
 * 一些变
 *  Created on: Sep 6, 2024
 *      Author: 24848
 */

#ifndef DEVICES_INC_UWB_CONSTS_H_
#define DEVICES_INC_UWB_CONSTS_H_

/* Default antenna delay values for 64 MHz PRF. See NOTE 1 below. */
//#define TX_ANT_DLY 16505
#define TX_ANT_DLY 16451

//#define RX_ANT_DLY 16505
#define RX_ANT_DLY 16451

#define PI 3.1415926

#define TBASE 				(100) //in us
//0.01Tb      ->        1us     时钟频率设置为 1MHz
#define MICRO_TB_NUM		(20)   //in  100us
#define UWB_REPLY_INTERVAL  (3)
#define MICRO_SLOT_US		(TBASE * MICRO_TB_NUM)

#define SUPERFRAME_TB_NUM	10000


# define DEFAULT_ANT_DELAY 16451
/* UWB microsecond (uus) to device time unit (dtu, around 15.65 ps) conversion factor.
 * 1 uus = 512 / 499.2 �s and 1 �s = 499.2 * 128 dtu. */
#define UUS_TO_DWT_TIME 65536   //  63898   us -> 转化成DW1000的1us的时间更加精确点似乎是  1us * 499.2e6*128 =
								//天线延迟？能否精确，是什么？从什么时候开始计算的？

//发送后进入接收的
#define POLL_TX_TO_RESP_RX_DLY_UUS	0


/* Receive final timeout. See NOTE 5 below. */
#define FINAL_RX_TIMEOUT_UUS 0      //2000*(UWB_REPLY_INTERVAL+1)
#define RESP_RX_TIMEOUT_UUS 0       //2000*(UWB_REPLY_INTERVAL+1)
#define ACK_RX_TIMEOUT_UUS 0		//2000*(UWB_REPLY_INTERVAL+1)
/**
 * @TODO
 * 时间
 */
#define POLL_RX_TO_RESP_TX_DLY_UUS  (MICRO_SLOT_US*UWB_REPLY_INTERVAL)    	//3000 这个是us单位
#define RESP_RX_TO_FINAL_TX_DLY_UUS (MICRO_SLOT_US*UWB_REPLY_INTERVAL)
#define FINAL_RX_TO_ACK_TX_DLY_UUS 	(MICRO_SLOT_US*UWB_REPLY_INTERVAL)

/* Preamble timeout, in multiple of PAC size. See NOTE 6 below. */
#define PRE_TIMEOUT 0

/* Speed of light in air, in metres per second. */
#define SPEED_OF_LIGHT 299702547


#define RXPACC_ADJUSTMENT (-18) //adjustment for decawave PRF length 16
#define A_PRF64M (121.74f)




#endif /* DEVICES_INC_UWB_CONSTS_H_ */
