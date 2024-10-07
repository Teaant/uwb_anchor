/*
 * mem_manager.h
 *
 *  Created on: Sep 29, 2024
 *      Author: 24848
 */

#ifndef UTILES_INC_MEM_MANAGER_H_
#define UTILES_INC_MEM_MANAGER_H_


#include "main.h"
#include "uwb_mac.h"

void init_mem_pool(void);

slot_alloc_node_t* memp_alloc(void);
void memp_free(slot_alloc_node_t* item);


#endif /* UTILES_INC_MEM_MANAGER_H_ */
