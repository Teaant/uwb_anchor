#ifndef __UWB_H_
#define __UWB_H_

#include <stdio.h>
#include <string.h>
#include <uwb_consts.h>

#include "deca_device_api.h"
#include "deca_regs.h"
#include "deca_spi.h"
#include "deca_port.h"
#include "dwio.h"
#include "main.h"

#include "uwb_ranging.h"

//Tanya_add
//This is a file of UWB PHY layer
//甚至128都是可以的了 ~
#define	TX_BUFFER_1		0
#define TX_BUFFER_2		256
#define TX_BUFFER_3		512
#define TX_BUFFER_4		768


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

	AoAParamTypeDef aoa_param[DWT_NUM_DW_DEV];
	float phase[4];  //phase in deg

} UWBDef;




int32_t uwbInit(uint16_t ID);

void setTxDoneCallback(txDoneCb callback);
//Tanya_add
void rxOkCallback_Ranging(const dwt_cb_data_t *cbData, UWBPortTypeDef *pports);
//TX ok callback
void txOkCallback(const dwt_cb_data_t *cbData, UWBPortTypeDef *pports);
//Tanya_add end
void rxOkCallback_PDoA(const dwt_cb_data_t *, UWBPortTypeDef *);
void rxToCallback(const dwt_cb_data_t *, UWBPortTypeDef *);
void rxErrCallback(const dwt_cb_data_t *, UWBPortTypeDef *);


void enable_ranging(void);
void enable_pdoa(void);
void disable_pdoa(void);


uint64 get_tx_timestamp_u64(UWBPortTypeDef *pports);
uint64 get_rx_timestamp_u64(UWBPortTypeDef *pports);
uint64_t get_sys_timestamp(UWBPortTypeDef *pports);

uint64_t getDeltaT(uint64_t,uint64_t);
uint64_t getSumT(uint64_t, uint64_t);


void UWB_Write_Tx_Buffer(uint8_t* pdata, uint8_t len);
void UWB_StartTx(uint8_t is_expect);

void UWB_Set_DelayTX(uint64_t tx_time, uint8_t addr, uint8_t len);

void enable_rx_with_timeout(uint16_t timeout);

void UWB_Schedule_Beacon_Frame(uint8_t *pdata, uint8_t len, uint64_t tx_time, uint8_t is_timeout, uint8_t bop);

void UWB_Issue_Beacon_Frame(uint8_t *pdata, uint8_t len);

void UWB_Configure_Resp_In_Buffer(uint8_t* pdata, uint8_t len, uint8_t addr, uint64_t tx_time);
void issue_resp(void);

void UWB_StartTx_in_Addr(uint8_t is_expect, uint8_t len, uint8_t addr);

void UWB_Anchor_Ack_Req(uint8_t* pdata, uint8_t len);

void UWB_Tag_Send_Ranging(uint8_t* pdata, uint8_t len, uint64_t tx_time);
void UWB_Tag_Req_Join(uint8_t* pdata, uint8_t len, uint64_t tx_time);

void UWB_Send(uint8_t * pdata, uint8_t len, If_Delay_t is_delayed, uint32_t tx_time, If_Expected_t is_expect);


void enable_pdoa(void);
void disable_pdoa(void);


float uwb_calculate_power(UWBPortTypeDef *pports);

float uwb_calculate_rx_power(uint16_t cir_pwr, uint16_t rxpacc, uint16_t rxpacc_nosat);
float uwb_calculate_fp_power(uint16_t fp_amp1, uint16_t fp_amp2, uint16_t fp_amp3, uint16_t rxpacc, uint16_t rxpacc_nosat);

void sliding_filter(float *distance);
float distance_compensate(float distance);
float array_average(float* array, uint8_t num);


#endif
