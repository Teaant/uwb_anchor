/*
 * modbus.h
 *
 *  Created on: Oct 17, 2024
 *      Author: 506
 */

#ifndef INC_MODBUS_H_
#define INC_MODBUS_H_

#include <stdint.h>

#include "agent.h"

// This file defines structure of modbus parameters which reflect correspond modbus address space
// for each modbus register type (coils, discreet inputs, holding registers, input registers)

# define BROADCAST_ID 255
#define Slave_adrr  0x01  //从站的地址


#define FIRST_NUM	20
#define SECOND_NUM	20
#define THIRD_NUM	(MAX_NODE- FIRST_NUM -SECOND_NUM)

#define NODE_VALUE_SIZE		10

#define IS_AVAILABLE(data)	((data&0xFF00)>>8)
#define GET_TAG_NUM(data)	(data&0x00FF)

//话说是这样的吗？

#define INPUT_MY_ID_START_ADDR     	(0)
#define INPUT_MY_ID_SIZE		   	(3)

#define INPUT_NODES_START_ADDR      (INPUT_MY_ID_START_ADDR + INPUT_MY_ID_SIZE)
#define INPUT_NODES_SIZE			(sizeof(Tag_Ranging_t)*FIRST_NUM /2)

#define INPUT_NODE_START_ADDR2		(INPUT_NODES_START_ADDR + INPUT_NODES_SIZE)
#define INPUT_NODE_SIZE2			(sizeof(Tag_Ranging_t)*SECOND_NUM /2)

#define INPUT_NODE_START_ADDR3		(INPUT_NODE_START_ADDR2 + INPUT_NODE_SIZE2)
#define INPUT_NODE_SIZE3			(sizeof(Tag_Ranging_t)*THIRD_NUM /2)

#define HOLD_MY_ID_START_ADDR      	(0)
#define HOLD_MY_ID_SIZE       	   	(4)


void parse_modbus_msg(uint8_t* pdata);

#endif /* INC_MODBUS_H_ */
