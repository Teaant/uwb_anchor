/*
 * modbus.c
 *
 *  Created on: 2021年4月11日
 *      Author: Administrator
 *
 */
#include "main.h"
#include "modbus.h"
#include "modbus_com.h"

#include "corecomm.h"

extern volatile holding_reg_params_t holding_data_share __attribute__((section(".shared")));

volatile holding_reg_params_t hold_data = {0,};
volatile input_reg_params_t input_data = {0,};


uint16_t input_start_addr_array[]={INPUT_MY_ID_START_ADDR, INPUT_NODES_START_ADDR,INPUT_NODE_START_ADDR2, INPUT_NODE_START_ADDR3};
uint16_t hold_start_addr_array[]={HOLD_MY_ID_START_ADDR};

#define input_datas_sum	(sizeof(input_start_addr_array)/2)
#define hold_datas_sum	(sizeof(hold_start_addr_array)/2)

uint8_t 	mbs_send_buff[ModbusMaxSize];

uint8_t*	p_parse = NULL;
uint8_t* 	p_send = mbs_send_buff;

extern UART_HandleTypeDef huart2;
extern CRC_HandleTypeDef hcrc;
//更改寄存器字节序
//输入：p_reg寄存器首地址，num寄存器数量
//返回：寄存器首地址
static uint8_t* change_reg_byte_order(uint8_t *p_reg, uint8_t num);

/***9.8 更改：1.调整字节序 宏定义，在把实际数据存入到结构体之前字节序转换  同理从结构体中读取数据也要字节序转换  调试成功
 *			2.函数化地址合法
 *			3.重新编写了数据更新update_hold和update_input, 从更新所有数据变成根据起始地址更新一段所需要的数据
 *
 *
 *9.26  找到实际的数据，对应到结构体中的数据，	更新
 * *********/

uint16_t calculate_CRC(uint8_t *_pBuf, uint16_t _usLen){

	uint16_t CRC_val=0xFFFF;
	uint8_t i, j;
	for(i=0; i< _usLen; i++){
		//进行异或
		CRC_val ^=*(_pBuf+i);
		for(j=0; j<8; j++){
			if((CRC_val&0x01)==1){
				CRC_val = (CRC_val >> 1)^0xA001;
			}else{
				CRC_val>>=1;
			}
		}
	}
	return (CRC_val&0x00FF)|(CRC_val&0xFF00);  //低八位+高八位
}

//收到更新之后调用该函数更新 ~
void update_hold(uint16_t start_addr){

}


//读取数据
//当前p_send指向数据位置？
void read_data(uint8_t func, uint16_t start_addr, uint16_t num){
	uint8_t* p_data;
	uint8_t i;
//	update_input(start_addr);  //这个hold数据不需要怎么办吗？
	switch(func){
		case MB_RD_HOLD:
			//保持型寄存器 保持寄存器的值不需要更新，只需要直接读取即可
			//memcpy(p_send,(((uint8_t*)&hold_data)+start_addr*2), num*2);
			p_data=(((uint8_t*)&hold_data)+start_addr*2) ;
			break;
		case MB_RD_INPUT:
			//memcpy(p_send,(((uint8_t*)&input_data)+start_addr*2), num*2);
			p_data=(((uint8_t*)&input_data)+start_addr*2) ;
			break;
	}
	//当前指针的位置应该处于数据部分起始
	for(i=0; i<num; i++){
		*(p_send)=*(p_data+1);
		*(p_send+1)=*(p_data);
		p_send+=2;
		p_data+=2;
	}
	if((func == MB_RD_INPUT) && (start_addr == INPUT_MY_ID_START_ADDR)&& input_data.available == 1){
		//unsupported
		input_data.available = 0;
		input_data.node_num = 0;
	}
}

/**
 * @TODO 查看地址的合法性？   wait modify
 */
uint8_t is_valid_addr(uint8_t func, uint16_t start_addr, uint16_t num){
	//保持型
//	uint8_t i;  //索引
//	switch(func){
//		case	MB_RD_HOLD:
//		case	MB_WR_HOLD:
//		case	MB_WR_Mul_Hold:
//			for(i=0; i<sizeof(hold_num_array)/2 ;i++){
//				if(hold_start_addr_array[i]==start_addr){
//					if(hold_num_array[i]==num){return 1;}
//				}
//			}//所有都遍历了之后没有这个地址或者是怎么样，就是不合法的地址了
//			break;
//		case 	MB_RD_INPUT:
//			for(i=0;i<sizeof(input_num_array)/2; i++){
//				if(input_start_addr_array[i]==start_addr){
//					//vico test begin
//					if(i==9 && input_num_array[i]==num){
//						return 1;}
//					//vico test end
//					if(input_num_array[i]==num){return 1;}
//				}
//			}
//			break;
//	}
//	return 0;
}


void parse_modbus_msg(uint8_t* pdata){

	uint8_t slave_addr, function;
	uint16_t start_addr, data_num, crc;
	uint8_t byte_num;

	p_parse = pdata;
	slave_addr = *(p_parse);
	p_parse += 1;  //位于function
	if(slave_addr == Slave_adrr || slave_addr == 0xFF){
		function = *(p_parse);
		p_parse +=1;  //指向功能码之后的字节
		start_addr = (uint16_t)((*p_parse)<<8)|(*(p_parse+1));
		p_parse +=2;
		data_num = (uint16_t)((*p_parse)<<8)|(*(p_parse+1));
		p_parse +=2;  //数据量之后的位置
		switch(function){
		case MB_RD_INPUT:
		case MB_RD_HOLD:
			p_send = mbs_send_buff;
			*(p_send) = Slave_adrr;
			*(p_send+1) = function;
			*(p_send+2) = data_num * 2;
			p_send += 3; //当前指向数据位置
			read_data(function, start_addr, data_num);   //当前已指向CRC位置
			crc= HAL_CRC_Calculate(&hcrc, (uint32_t)mbs_send_buff, 2+1+data_num*2);
			//calculate_CRC(mbs_send_buff, 2+1+data_num*2);
//			p_send += data_num*2;
			*(p_send) = crc&0x00FF;
			*(p_send+1) = (crc&0xFF00)>>8;
			MB_Send(mbs_send_buff,2+1+data_num*2+2);
			//如何要in-avalable掉？
			if(start_addr == INPUT_NODES_START_ADDR){
				input_data.available = 0;
			}
			break;
		case MB_WR_HOLD:
			//字节数
			byte_num = *(p_parse);
			p_parse += 1;
			if(byte_num != data_num *2) break;
			memcpy((uint8_t*)((uint8_t*)&hold_data+start_addr*2), p_parse, byte_num);
			//vico add begin
			//数据是小端方式存储的，但是按照大端字节序传递的，更换字节序
			change_reg_byte_order((uint8_t*)&hold_data + start_addr*2, data_num);
			//vico add end
			//响应
			p_send = mbs_send_buff;
			memcpy(p_send, pdata, 2+4);
			p_send += 2+4;
			crc= HAL_CRC_Calculate(&hcrc, (uint32_t)mbs_send_buff,2+4+2);
			//calculate_CRC(mbs_send_buff, 2+1+data_num*2);
			//vico add begin
			*(p_send) = crc&0x00FF;//低八位
			*(p_send+1) = (crc&0xFF00)>>8;
			MB_Send(mbs_send_buff,2+4+2);
			update_hold(start_addr);
			break;
		}
	}
}

////发送错误响应帧
//void send_err_msg(uint8_t err_code){
//
//	uint16_t crc;
//	p_send=mbs_send_buff;
//	memcpy((uint8_t*)(&send_frame), mbs_rcv_buff, sizeof(MB_header)); //复制头部
//	send_frame.header.MB_Func_t=send_frame.header.MB_Func_t|0x80; //差错号
//	send_frame.payload.send_rd_msg.length_of_byte=err_code;
//	//计算校验
//	memcpy(p_send, (uint8_t*)&send_frame, sizeof(MB_header)+1);  //总觉得再从结构体复制数据有点多余  但是就先这样吧
//	p_send+=sizeof(MB_header)+1;
//	crc=calculate_CRC(mbs_send_buff,sizeof(MB_header)+1);
//	*p_send=crc>>8;
//	p_send++;
//	*p_send=crc&0x00FF;
//	MB_Send(mbs_send_buff,sizeof(MB_header)+3);
//
//}

void MB_Send(uint8_t *_pBuf, uint8_t length){
//
//	if(osSemaphoreAcquire(ESP32IdleSem, osWaitForever) == osOK)
//	{
		HAL_UART_Transmit_DMA(&huart2, _pBuf, length);
		//HAL_UART_Transmit(&huart2, _pBuf, length, 100);
//		osSemaphoreRelease(ESP32IdleSem);
//	}
}

//vico add begin
static uint8_t* change_reg_byte_order(uint8_t *p_reg, uint8_t num){
	uint8_t* p_head = p_reg;
	uint8_t tmp;
	for (uint8_t i = 0; i < num; ++i){
		tmp = *p_reg;
		*p_reg = *(p_reg+1);
		*(++p_reg) = tmp;
		++p_reg;
	}
	return p_head;
}





