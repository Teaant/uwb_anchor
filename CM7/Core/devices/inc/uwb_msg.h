/*
 * uwb_msg.h
 *
 *  Created on: May 26, 2024
 *      Author: 24848
 */

#ifndef DEVICES_INC_UWB_MSG_H_
#define DEVICES_INC_UWB_MSG_H_


#include "uwb.h"

#define MAX_TAG    51

typedef enum{

	beacon_frame = 0,
	data_frame = 1,
	ack_frame = 2,
	mac_cmd_frame = 3,
	compound_frame = 5,

}Frame_Type_t;

typedef enum{
	//定位
	UWB_Ranging_Poll = 0,
	UWB_Ranging_Resp,
	UWB_Ranging_Final,
	UWB_Ranging_Ack,

	UWB_App_Data,

	UWB_Cmd_Req = 0x11,
	UWB_Cmd_Modify = 0x12,
	UWB_Cmd_Release,

}UWB_Ranging_Msg_Type;

typedef enum{
	initiator = 0,
	responder = 1,
}UWB_Ranging_Role;

/**
 * 定义消息结构体
 */

/*
 * 802.15.4 帧头部
 *|     帧控制     | 序列号 |   目标PAN ID  |   目标地址  |  源地址    |  xx      payload  |  FCS   |
 *  2 + 1 +  2 +  2 + 2 = 9
 * */
//802.15.4前两字节为帧控制字段
/*
 * 帧类型    b2-0             001  数据帧   000    101
 * 安全使能   b3              0
 * 帧Pending   b4			0
 * 需要ACK  b5				0
 * PAN ID 压缩   b6			1     //1表示只有Dest PAN ID,0两者都有
 * 保留      b7-9			000
 * 目标地址模式   b10-11		10  目标地址是短地址
 * 帧版本         b12-13      00
 * 源地址模式      b14-15      10 源地址为短地址
 * */
//----> 两个字节的Frame control :  0x88(暂时高字节可以固定)
//发送，低字节在前

#define UWB_MAC_HEADER_LEN		9

#define ALL_MSG_COMMON_LEN 		9
#define ALL_MSG_ADD_LEN         2
#define ALL_MSG_DIST_PAN_IDX    3
#define ALL_MSG_DIST_IDX        5
#define ALL_MSG_SRC_IDX     	7

#define FRAME_CONTROL_HIGH		0x88
#define FRAME_CONTROL_LOW		0x41

//难道不对吗？不知是否是因为字节顺序不太对哈 ？
#define FRAME_CONTROL			0x8841

#define	FRAME_TYPE_CUS			0x0380

/**
 * 设置帧类型字段
 * 设置需要ACK请求
 */

#define SET_FRAMECONTROL(type, ack)		(FRAME_CONTROL|((type)&0x07)|((ack)&0x0020))

//#define GET_FRAMETYPE(control)			(control&0x07)

#define BEACON_CONTROL					(FRAME_CONTROL)
#define POLL_CONTROL					(FRAME_CONTROL|(0x0001<<7))
#define RESP_CONTROL					(FRAME_CONTROL|(0x0001<<7))
#define FINAL_CONTROL					(FRAME_CONTROL|(0x0001<<7))    //不需要响应的


//#define BEACON_CONTROL					(0x8840)
//#define POLL_CONTROL					(0x8841)
//#define RESP_CONTROL					(0x8841)
//#define FINAL_CONTROL					(0x8841)


#define POLL_COMP_CONTROL				(FRAME_CONTROL|(0x0005<<7))
#define RESP_COMP_CONTROL				(FRAME_CONTROL|(0x0005<<7))
#define FINAL_COMP_CONTROL				(FRAME_CONTROL|(0x0005<<7))


#define MAC_CMD_CONTROL					(FRAME_CONTROL|(0x0003<<7))
#define ACK_FRAME_CONTROL				(FRAME_CONTROL|(0x0002<<7))
//
//#define MAC_CMD_CONTROL					(0x8843)
//#define ACK_FRAME_CONTROL				(0x8842)

#define GET_FRAMETYPE(control)			((control&FRAME_TYPE_CUS)>>7)
//#define GET_FRAMETYPE(control)			(control&0x0007)

//数据包载荷的字节大小
#define FINAL_PAYLOAD_LEN		(24+1+2)	//function代码
//#define ACK_PAYLOAD_LEN			(32+1+2)    //2是FCS的长度，需要包含在内
#define POLL_PAYLOAD_LEN		(0+1+2)
#define RESP_PAYLOAD_LEN		(0+1+2)

#define	JOIN_REQ_PAYLOAD_LEN	(1+1+2)

#define POLL_MSG_LEN			(UWB_MAC_HEADER_LEN + POLL_PAYLOAD_LEN)
#define RESP_MSG_LEN			(UWB_MAC_HEADER_LEN + RESP_PAYLOAD_LEN)
#define FINAL_MSG_LEN			(UWB_MAC_HEADER_LEN + FINAL_PAYLOAD_LEN)
//#define ACK_MSG_LEN				(UWB_MAC_HEADER_LEN + ACK_PAYLOAD_LEN)

#define BEACON_COM_LEN			(UWB_MAC_HEADER_LEN+ 3 + 2)

#define JOIN_REQ_MSG_LEN		(UWB_MAC_HEADER_LEN + JOIN_REQ_PAYLOAD_LEN)
#define JOIN_RESPONSE_LEN		(UWB_MAC_HEADER_LEN + 2) //还有两个是FCS

#pragma pack(1)

/**
 * @TODO
 * 暂时采用固定头部
 */

//修改这个frame control 里面的保留字节吧，对应到这个数据包的类型，现在只能过滤数据帧相当于是，万幸的是保留了地址过滤
typedef struct{
	uint16_t control;
	uint8_t sequence;
	uint16_t pan_id;
	uint16_t dist;
	uint16_t src;
}UWB_Msg_Header_t;   //9

typedef struct{
	uint64	poll_tx_ts;
	uint64	resp_rx_ts;
	uint64	final_tx_ts;
}UWB_Ranging_Final_t;   //24

//typedef struct{
//	int64	tof_dtu;
//	int64	D1;
//	int64	D2;
//	int64	D3;
//}UWB_Ranging_ACK_t;   //32

/**
 * @TODO beacon帧:
 * finish
 */

//只需包含宏时隙号
/**
 * 锚节点号	信号强度	宏时隙号	有效性	微时隙号（4*（宏时隙号-1）+1）	微时隙号（4*（宏时隙号-1）+4）	微时隙号（4*（宏时隙号-1）+7）	微时隙号（4*（宏时隙号-1）+10）
	2字节	2字节	2字节	1字节	2字节	2字节	2字节	2字节
 *
 */

/**
 * 宏时隙和微时隙对应
 * 第几组  第几个
 *1: 1  4  7  10		((宏时隙-1)/3)*3+((宏时隙-1)%3)+ 1,3,6,9
 *2: 2  5  8  11
 *3: 3  6  9  12
 *
 *4: 13 16 19 22
 *5: 14 17 20 23
 *6: 15 18 21 24
 *
 *((macro-1)/3)*12 +(macro-1)%3+1
 *故目前响应时间Treply只有 2 * 微时隙


//固定超帧的长度500ms or 1000ms
//可能会有很长一段时间处于
微时隙周期 M * Tb   			1B
CFP宏时隙数目  p				1B

CAP数目						1B
CAP开始微时隙号 3p + 1
125 - 9 - 2 = 114
114 /2 = 57
//最多只能50几个节点 数据包长度限制
*/

typedef struct{
	uint8_t period;        //或可更长呢？
	/**
	 * @TODO 如果这样的话，那个要换成变量而不是宏定义形式
	 */
	uint8_t CFP_num;  //对应IDs的数量
	uint8_t CAP_num; //
	uint16_t IDs[MAX_TAG];
}UWB_Beacon_Payload_t;

typedef struct{
	UWB_Msg_Header_t header;
	UWB_Beacon_Payload_t payload;
}UWB_Beacon_Frame_t;

typedef struct{
	uint8_t function;
	UWB_Ranging_Final_t  final_payload;
}UWB_Function1_t;

//数据帧
typedef struct{

	UWB_Msg_Header_t header;
	UWB_Function1_t payload1;

}UWB_Data_Frame_t;

//MAC命令帧 —— 用于时隙管理
typedef struct{
	UWB_Msg_Header_t header;
	uint8_t function;
	uint8_t interval;
}UWB_Mac_Frame_t;

//分层  消息队列？
/**
 * 复合帧
 */

typedef struct{
	uint8_t function;
	//127 -7 -2(FCS) - 1- 32 -1 = 84
	uint8_t data[84];
}UWB_Function2_t;

typedef struct{

	UWB_Msg_Header_t header;
	UWB_Function1_t payload1;
	UWB_Function2_t payload2;

}UWB_Comp_Frame_t;

#pragma pack()


#endif /* DEVICES_INC_UWB_MSG_H_ */
