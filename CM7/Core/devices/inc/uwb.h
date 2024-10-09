#include <stdio.h>
#include <string.h>

#include "deca_device_api.h"
#include "deca_regs.h"
#include "deca_spi.h"
#include "deca_port.h"
#include "dwio.h"
#include "main.h"

#include "uwb_consts.h"
#include "uwb_ranging.h"

//Tanya_add
//This is a file of UWB PHY layer

#ifndef __UWB_H_
#define __UWB_H_

typedef void(*txDoneCb)(void);

typedef enum{
	Buffer_ready = 0,
	Buffer_busy
}Radio_TX_State_t;

typedef struct
{
	UWBPortTypeDef ports[DWT_NUM_DW_DEV];
	//UWB add
	uint16_t antDelay;
	uint16_t err_time;

} UWBDef;


int32_t uwbInit(uint16_t ID, uint8_t role);

void setTxDoneCallback(txDoneCb callback);
//Tanya_add
void rxOkCallback_Ranging(const dwt_cb_data_t *cbData, UWBPortTypeDef *pports);
//TX ok callback
void txOkCallback(const dwt_cb_data_t *cbData, UWBPortTypeDef *pports);
//Tanya_add end
void rxOkCallback_PDoA(const dwt_cb_data_t *, UWBPortTypeDef *);
void rxToCallback(const dwt_cb_data_t *, UWBPortTypeDef *);
void rxErrCallback(const dwt_cb_data_t *, UWBPortTypeDef *);




uint64 get_tx_timestamp_u64(UWBPortTypeDef *pports);
uint64 get_rx_timestamp_u64(UWBPortTypeDef *pports);

uint64_t getDeltaT(uint64_t,uint64_t);


void UWB_Write_Tx_Buffer(uint8_t* pdata, uint8_t len);
void UWB_StartTx(uint8_t is_expect);
void UWB_Send(uint8_t * pdata, uint8_t len, If_Delay_t is_delayed, uint32_t tx_time, If_Expected_t is_expect);



float uwb_calculate_power(UWBPortTypeDef *pports);

float uwb_calculate_rx_power(uint16_t cir_pwr, uint16_t rxpacc, uint16_t rxpacc_nosat);
float uwb_calculate_fp_power(uint16_t fp_amp1, uint16_t fp_amp2, uint16_t fp_amp3, uint16_t rxpacc, uint16_t rxpacc_nosat);

void sliding_filter(float *distance);
float distance_compensate(float distance);
float array_average(float* array, uint8_t num);


#endif
