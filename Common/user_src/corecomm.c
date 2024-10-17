/*
 * corecomm.c
 *
 *  Created on: 2020年11月3日
 *      Author: xsxsx
 */


#include "corecomm.h"

//extern int32_t *pBuffer[8] __attribute__ ((section(".shared")));
//extern int32_t coreBuffer[8][32] __attribute__ ((section(".shared")));

//extern int32_t *pBuffer[8];
volatile  int8_t coreBuffer[16][128];
volatile  CoreCommDef CoreComm;


int32_t bufferInit()
{
	CoreComm.genInterrupt=genInterrupt;
	CoreComm.readBuffer=readBuffer;
	CoreComm.writeBuffer=writeBuffer;
	CoreComm.actInterrupt=actInterrupt;
	memset(CoreComm.txBuffer,0,sizeof(CoreComm.txBuffer));
	memset(CoreComm.rxBuffer,0,sizeof(CoreComm.txBuffer));
	return 0;
}
int32_t writeBuffer(void *pCoreBuffer, int len)
{
	memcpy(pCoreBuffer,CoreComm.txBuffer,len);
	memset(CoreComm.txBuffer,0,len);
	return 0;
}
int32_t readBuffer(void *pCoreBuffer, int len)
{
	memcpy(CoreComm.rxBuffer,pCoreBuffer,len);
	memset(pCoreBuffer,0,len);
	return 0;
}
int32_t genInterrupt(int32_t n)
{
//	actCM7Interrupt(n);
	HAL_HSEM_FastTake(n);
	HAL_HSEM_Release(n, 0);
	return 0;
}
int32_t actInterrupt(int32_t n)
{
	HAL_HSEM_ActivateNotification(__HAL_HSEM_SEMID_TO_MASK(n));
	return 0;
}
int32_t actCM7Interrupt(int32_t n)
{
	uint32_t SemMask;
	SemMask=__HAL_HSEM_SEMID_TO_MASK(n);
	HSEM->C1IER |= SemMask;

}

int32_t actCM4Interrupt(int32_t n)
{
	uint32_t SemMask;
	SemMask=__HAL_HSEM_SEMID_TO_MASK(n);
	HSEM->C2IER |= SemMask;
}
