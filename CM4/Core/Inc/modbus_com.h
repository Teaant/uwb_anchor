/*
 * modbus_com.h
 *
 *  Created on: Oct 17, 2024
 *      Author: 24848
 */

#ifndef INC_MODBUS_COM_H_
#define INC_MODBUS_COM_H_


#define ModbusMaxSize	256

enum MB_Func_t{

	MB_RD_INPUT=4,  //读取输入寄存器
	MB_RD_HOLD=3,   //读取保持寄存器
	MB_WR_HOLD=6,   //写保持寄存器  1个
	MB_WR_Mul_Hold=0x10,  //写多个保持寄存器

};

enum MB_err_code{
	err_func =0x01,  //功能码不支持
	err_addr=0x02,  //初始地址和初始地址+寄存器数量  错误
	err_data_num=0x03, //寄存器的数量错误
	err_ok=0x0A,   //正常情况
};

//建议增加发送接收缓冲区，uint8_t mbs_send_buff[256],mbs_rcv_buff[256]
//解析非固定长度数据帧时会方便很多，以后对MB_header之类结构体更改也不用重写代码。
//MB_FrameTypeDef rcv_frame;
//uint8_t* p_parse = mbs_rcv_buff;
//rcv_frame.header = *(MB_header*)p_parse;
//p_parse+=sizeof(MB_header);
//memcpy(rcv_frame.payload, p_parse, 帧头里的字节数)    ??帧头中的字节数  自己定义的吗？
//p_parse+=帧头里的字节数
//rcv_frame.crc = *(mb_crc*)p_parse;
//p_parse+=sizeof(rcv_frame.crc);

# pragma pack()

void MB_Send(uint8_t *_pBuf, uint8_t length);   //发送
uint16_t calculate_CRC(uint8_t *_pBuf, uint16_t _usLen);  //计算CRC
//void StartParseModbusTask(void *arg);
void send_err_msg(uint8_t err_code);
void read_data(uint8_t func, uint16_t start_addr, uint16_t num);
uint8_t is_valid_addr(uint8_t func, uint16_t start_addr, uint16_t num);   //检验地址的合法性

void update_hold(uint16_t start_addr);  //根据起始地址更新输入寄存器中的数据内容，更新之后读取
//void update_input(uint16_t start_addr);

void parse_msg_to_agent();


#endif /* INC_MODBUS_COM_H_ */


