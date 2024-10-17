/*
 * mem_manager.c
 *
 *  Created on: Sep 29, 2024
 *      Author: 24848
 */

#include "mem_manager.h"


#define NODE_ITEM_SIZE	sizeof(slot_alloc_node_t)

static uint64_t alloc_mask = 0;
static slot_alloc_node_t 	node_mem_pool[64];
//也许可以每一次把最低为的这个记录下来，这样便可以不用再找了，或者记录最新的这个pos
static uint8_t alloc_pos = 0;

static int8_t find_free_pos(void);


void init_mem_pool(void){
	alloc_mask = 0;
	alloc_pos = 0;
}

slot_alloc_node_t* memp_alloc(void){
	int8_t pos = find_free_pos();
	if(pos >= 0){
		alloc_mask &= 0x00000001 << pos;
//		node_mem_pool[pos].pnext = NULL;
		return &node_mem_pool[pos];
	}else {
		return NULL;
	}
}

void memp_free(slot_alloc_node_t* item){

	uint8_t pos = item - node_mem_pool;
	memset((uint8_t*)&node_mem_pool[pos], 0, sizeof(slot_alloc_node_t));
	alloc_mask &= ~(1<<pos);

}

static int8_t find_free_pos(void){
	uint8_t pos = alloc_pos + 1;   //从上一次已经分配出去的位置开始 ~
	uint8_t k ;
	for(k = 0; k < 64; k ++){
		pos = (pos + k)%64;  //避免溢出 ~
		if((alloc_mask&(0x00000001<<pos)) == 0){
			alloc_pos = pos;
			return pos;
		}
	}
	alloc_pos = -1;
	return -1;
}
