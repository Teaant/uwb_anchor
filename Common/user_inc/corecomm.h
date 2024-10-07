#ifndef __CORECOMM_H__
#define __CORECOMM_H__

#include "main.h"
#include <stdlib.h>
#include <string.h>
//HSEM
#define GET_TARGET_ANG_HSEM 20
#define GET_ACC_MEM_HSEM 21
#define END_ACC_MEM_HSEM 22
#define AOA_CALI_COMMAND_HSEM 23
#define AOA_CALI_RESULT_HSEM 24

#define I2C2_BUSY_HSEM 30
#define ENABLE_CM4_HSEM 31
#define IMU_FUSION_COMPLETE_HSEM 5
#define IMU_TEST_MAG_HSEM 3
//BUFFER
#define TARGET_ANG_BUFFER 10
#define ACC_MEM_BUFFER 11
#define AOA_CALI_COMMAND_BUFFER 12
#define AOA_CALI_RESULT_BUFFER 13

#define IMU_FUSION_BUFFER 5
#define IMU_TEST_MAG_BUFFER 3

typedef int32_t (*writeBuffer_t) (void*,int);
typedef int32_t (*readBuffer_t) (void*,int);
typedef int32_t (*genInterrupt_t) (int32_t);
typedef int32_t (*actInterrupt_t) (int32_t);

typedef struct
{
	writeBuffer_t writeBuffer;
	readBuffer_t readBuffer;
	genInterrupt_t genInterrupt;
	actInterrupt_t actInterrupt;
	int8_t txBuffer[128];
	int8_t rxBuffer[128];
}CoreCommDef;

typedef struct
{
	uint16_t ID;
	int64_t TofDtu;
	int64_t D1;
	int64_t D2;
	int64_t D3;
	uint64_t  t1;
	uint64_t t2;
	uint64_t t3;
	uint64_t t4;
}RxAckToCM7Def;

typedef struct{
	float phase;
	float theta;
	float alpha;
	float normalized_amp;
	float raw_amp;
	uint16_t src_car_id;
	uint8_t avalible;
}AoADataTypeDef;

typedef struct{
	uint8_t my_dw_id;
	uint8_t rcphase;
	uint32_t rx_ts;
	float fp_angle;
	uint16_t fp_index;
	uint16_t fp_amp1;
	uint16_t fp_amp2;
	uint16_t fp_amp3;
	uint32_t fp_amp_sum;
	uint16_t std_noise;
	uint8_t avalible;
	uint16_t rxpacc;
	uint16_t rxpacc_nosat;
	uint16_t cir_pwr;
}AoADiagnosticTypeDef;

int32_t bufferInit();
int32_t writeBuffer(void*, int);
int32_t readBuffer(void*, int);
int32_t genInterrupt(int32_t);
int32_t actInterrupt(int32_t);
int32_t actCM7Interrupt(int32_t n);
int32_t actCM4Interrupt(int32_t n);

#endif
