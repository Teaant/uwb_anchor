#ifndef __UWB_H_
#define __UWB_H_


#include "deca_device_api.h"
#include "deca_regs.h"
#include "deca_spi.h"
#include "deca_port.h"
#include "dwio.h"
#include "main.h"




typedef struct
{
	UWBPortTypeDef ports[DWT_NUM_DW_DEV];
	//UWB add
	uint16_t antDelay;
	uint16_t err_time;

//	AoAParamTypeDef aoa_param[DWT_NUM_DW_DEV];
//	float phase[4];  //phase in deg

} UWBDef;




int32_t uwbInit(uint16_t ID);

//TX ok callback
void txOkCallback(const dwt_cb_data_t *cbData, UWBPortTypeDef *pports);
//Tanya_add end
void rxOkCallback_PDoA(const dwt_cb_data_t *, UWBPortTypeDef *);
void rxToCallback(const dwt_cb_data_t *, UWBPortTypeDef *);
void rxErrCallback(const dwt_cb_data_t *, UWBPortTypeDef *);

void process_pdoa(uint16_t id);

void enable_pdoa(void);
void disable_pdoa(void);


uint64 get_tx_timestamp_u64(UWBPortTypeDef *pports);
uint64 get_rx_timestamp_u64(UWBPortTypeDef *pports);

uint64_t getDeltaT(uint64_t,uint64_t);
uint64_t getSumT(uint64_t, uint64_t);


float uwb_calculate_power(UWBPortTypeDef *pports);

float uwb_calculate_rx_power(uint16_t cir_pwr, uint16_t rxpacc, uint16_t rxpacc_nosat);
float uwb_calculate_fp_power(uint16_t fp_amp1, uint16_t fp_amp2, uint16_t fp_amp3, uint16_t rxpacc, uint16_t rxpacc_nosat);



#endif
