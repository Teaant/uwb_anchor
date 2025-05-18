/*
 * uwb_consts.h
 * 一些变
 *  Created on: Sep 6, 2024
 *      Author: 24848
 */

#ifndef USER_INC_UWB_CONSTS_H_
#define USER_INC_UWB_CONSTS_H_

/* Default antenna delay values for 64 MHz PRF. See NOTE 1 below. */
//#define TX_ANT_DLY 16505
#define TX_ANT_DLY 16401

//#define RX_ANT_DLY 16505
#define RX_ANT_DLY 16401

#define PI 3.1415926
#define PI2 	6.2831852

//通信距离？ ~
#define 	COMM_RANGING	3

# define DEFAULT_ANT_DELAY 16451

/* UWB microsecond (uus) to device time unit (dtu, around 15.65 ps) conversion factor.
 * 1 uus = 512 / 499.2 �s and 1 �s = 499.2 * 128 dtu. */
#define UUS_TO_DWT_TIME 	0x000000000000F99A    //  63898   us -> 转化成DW1000的1us的时间更加精确点似乎是  1us * 499.2e6*128 =
#define MS_TO_DWT_TIME		0x0000000003CF0000 	//63897600    //
#define S_TO_DWT_TIME		0x0000000EE0980000	//63897600000  EE0980000

//1099511627775

//后续可能会用到的
#define MS_2	0x00000000079E0000 //127795200
#define MS_4	0x000000000F3C0000 //255590400	//BOP 1 3
#define MS_5	0x00000000130B0000 //319488000   //BOP 0 2
#define MS_6	0x0000000016DA0000 //383385600
#define MS_9	0x0000000022470000 //575078400
#define MS_12	0x000000002DB40000
#define MS_13	0x0000000031830000
#define MS_14	0x0000000035520000
#define MS_18	0x00000000448E0000

#define RX_TIMEOUT_4_2MS	4096
#define RX_TIMEOUT_4_5MS 	4388     //every poll
#define RX_TIMEOUT_10MS 	9756	//BOP 0, 2
#define RX_TIMEOUT_8MS  	7805   //BOP 1,3

#define RX_ENABLE_DELAYED_MS	0

#define TX_TIME_MASK	0xFFFFFFFFFFFFFE00

//传输延迟：100/3/1000000000 = 333ns   以100m，10m就是33.3ns,应该已经还是挺小的了，按道理来说这个时候的同步误差就是几十、一百多个ns

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
#define RESP_RX_TO_FINAL_TX_DLY_UUS (4000)   //in us
#define FINAL_RX_TO_ACK_TX_DLY_UUS 	(MICRO_SLOT_US*UWB_REPLY_INTERVAL)

/* Preamble timeout, in multiple of PAC size. See NOTE 6 below. */
#define PRE_TIMEOUT 0

/* Speed of light in air, in metres per second. */
#define SPEED_OF_LIGHT 299702547

#define UWB_CARRIER_FREQ_CH2 		(4.0e9f)
//#define LAMDA_M 					(SPEED_OF_LIGHT / UWB_CARRIER_FREQ_CH2)	//wave length of carrier wave
#define LAMDA_M						(0.0749256f)
#define D_M							(0.02f)   //distance between two antennas
#define ND_M						(-0.02f)   //distance between two antennas


#define RXPACC_ADJUSTMENT (-18) //adjustment for decawave PRF length 16
#define A_PRF64M (121.74f)


#endif /* USER_INC_UWB_CONSTS_H_ */
