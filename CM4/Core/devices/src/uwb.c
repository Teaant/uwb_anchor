#include "uwb.h"
#include "corecomm.h"
#include "main.h"
# include "agent.h"
#include <math.h>

#include "gpio.h"

#include "uwb_msg.h"
#include "uwb_consts.h"



//#define ACC_MEM_TEST 1
/* Default communication configuration. We use here EVK1000's default mode (mode 3). */

/**
 */
static dwt_config_t config = {
	2, /* Channel number. */
	DWT_PRF_64M, /* Pulse repetition frequency. */
	DWT_PLEN_1024, /* Preamble length. Used in TX only. */
	DWT_PAC32, /* Preamble acquisition chunk size. Used in RX only. */
	9, /* TX preamble code. Used in TX only. */
	9, /* RX preamble code. Used in RX only. */
	0, /* 0 to use standard SFD, 1 to use non-standard SFD. */
	DWT_BR_850K, /* Data rate. */
	DWT_PHRMODE_STD, /* PHY header mode. */
	(1025 + 64 - 32) /* SFD timeout (preamble length + 1 + SFD length - PAC size). Used in RX only. */
};

/* Declaration of static functions. */



extern volatile AoADataTypeDef aoa_data[MAX_TAG];

extern volatile PDoA_Struct_t pdoa_diags[3];

extern volatile uint8_t rx_fail ;

extern volatile uint32_t error_status;


UWBDef UWB = { .err_time = 0, .antDelay = DEFAULT_ANT_DELAY};

extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim7;


extern SPI_HandleTypeDef hspi1;
extern SPI_HandleTypeDef hspi2;
extern SPI_HandleTypeDef hspi3;
extern SPI_HandleTypeDef hspi4;


extern volatile PDoA_Frame_t rxBuffer;

static void initOtherPorts(void);

static void anchor_parse_pdoa( UWBPortTypeDef *pports, uint16_t id, uint8_t index);
static void anchor_parse_pdoa_final(UWBPortTypeDef *pports, uint16_t id, uint8_t index);

extern volatile uint8_t flag;

extern volatile uint8_t ranging_num;
/**
 * @TODO done 物理层的初始化
 * SPI， 帧过滤相关等
 */
int32_t uwbInit(uint16_t ID) {

	//min : C0   max:1F	 //with 85 default and nothing special ~
//	dwt_txconfig_t config_t = {
//			.PGdly = 0xC2,
//			.power = 0x851F1F85
//	}; //1110 0000

	initOtherPorts();

	for (int i = 1; i < DWT_NUM_DW_DEV; i++){
		reset_DW1000(&UWB.ports[i]);
		UWB.ports[i].avalible = 0;
	}

	for (int i = 1; i < DWT_NUM_DW_DEV; i++) {

		UWBPortTypeDef *pports = &UWB.ports[i];
		int trys = 10;
		port_set_dw1000_slowrate(pports);
		dwt_setlocaldataptr(i);
		do {
			if (dwt_initialise(DWT_LOADUCODE, pports) == DWT_SUCCESS) {
				UWB.ports[i].avalible = 1;
			} else {
				trys--;
			}
		} while ((trys > 0) && UWB.ports[i].avalible != 1);
		if (trys <= 0) {
			continue;
		}
		/**
		 * dwt_enableframefilter()
		 *  * @brief This is used to enable the frame filtering - (the default option is to
		 * accept any data and ACK frames with correct destination address
		 */

		dwt_setpanid(PAN_ID, pports);
		dwt_setaddress16(MY_ID, pports);
		dwt_enableframefilter(DWT_FF_DATA_EN | DWT_FF_RSVD_EN, pports);
//    	dwt_enableframefilter(SYS_CFG_FF_ALL_EN, pports);

		port_set_dw1000_fastrate(pports);

		/*Setup Interrupt*/
		dwt_setinterrupt((SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_ERR| SYS_STATUS_ALL_RX_TO), 2, pports);

		dwt_setcallbacks(NULL, rxOkCallback_PDoA, rxToCallback, rxErrCallback);

		port_set_deca_isr(dwt_isr);

		dwt_write32bitreg(SYS_STATUS_ID,
				SYS_STATUS_ALL_RX_GOOD|SYS_STATUS_ALL_RX_TO|SYS_STATUS_ALL_RX_ERR,
				pports);
		/* Configure DW1000. See NOTE 7 below. */
		dwt_configure(&config, pports);

//		//2024.06.03  发射功测试
//		dwt_setsmarttxpower(0, pports);
//		//设置发射功率
//		dwt_configuretxrf(&config_t, pports);
//		//end

		/* Apply default antenna delay value. See NOTE 1 below. */
		dwt_setrxantennadelay(RX_ANT_DLY, pports);
		dwt_settxantennadelay(TX_ANT_DLY, pports);

		/* Set preamble timeout for expected frames. See NOTE 6 below. */
		dwt_setpreambledetecttimeout(PRE_TIMEOUT, pports);
		/* Configure LED */
		dwt_setleds(1, pports);
		/* Set Sync to OSTR*/
		dwt_setecctrl(EC_CTRL_OSTRM, pports);
	}
	return 0;
}


uint64 get_tx_timestamp_u64(UWBPortTypeDef *pports) {
	uint8 ts_tab[5];
	uint64 ts = 0;
	int i;
	dwt_readtxtimestamp(ts_tab, pports);
	for (i = 4; i >= 0; i--) {
		ts <<= 8;
		ts |= ts_tab[i];
	}
	return ts;
}

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn get_rx_timestamp_u64()
 *
 * @brief Get the RX time-stamp in a 64-bit variable.
 *        /!\ This function assumes that length of time-stamps is 40 bits, for both TX and RX!
 *
 * @param  none
 *
 * @return  64-bit value of the read time-stamp.
 */
uint64 get_rx_timestamp_u64(UWBPortTypeDef *pports) {
	uint8 ts_tab[5];
	uint64 ts = 0;
	int i;
	dwt_readrxtimestamp(ts_tab, pports);
	for (i = 4; i >= 0; i--) {
		ts <<= 8;
		ts |= ts_tab[i];
	}
	return ts;
}

static uint64_t get_sys_timestamp(UWBPortTypeDef *pports) {
	uint8 ts_tab[5];
	uint64 ts = 0;
	int i;
	dwt_readsystime(ts_tab, pports);
	for (i = 4; i >= 0; i--) {
		ts <<= 8;
		ts |= ts_tab[i];
	}
	return ts;
}

void rxOkCallback_PDoA(const dwt_cb_data_t* cdata, UWBPortTypeDef *pports) {

	//有这个frame_control的类
	uint8_t pdoa_id = pports - UWB.ports;
	uint8_t* pdata = (uint8_t*)&rxBuffer;
	dwt_readrxdata(pdata, UWB_MAC_HEADER_LEN + 1, 0, pports);

	uint8_t frame_type = GET_FRAMETYPE(rxBuffer.header.control);

	if (frame_type != data_frame || rxBuffer.header.dist != MY_ID ){
		/* Activate reception immediately. */
		dwt_rxenable(DWT_START_RX_IMMEDIATE , pports);
		return;
	}
	if( rxBuffer.function == UWB_Ranging_Poll){
		anchor_parse_pdoa(pports, rxBuffer.header.src, pdoa_id);
	}
#if(USE_TWO_PDOA)
	else if( rxBuffer.function == UWB_Ranging_Final){
		anchor_parse_pdoa_final(pports, rxBuffer.header.src, pdoa_id);
	}
#endif
	dwt_rxenable(DWT_START_RX_IMMEDIATE , pports);
	//也不知这段时间需要多少？

//	uint64_t time = get_sys_timestamp(pports);

}

void rxErrCallback(const dwt_cb_data_t *cbData, UWBPortTypeDef *pports) {
	//其实这边如果出现什么接受错误，应该按照当前的状态做一些改动的 ~
	/* Clear reception timeout to start next ranging process. */
//	HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
//	rx_fail++;
//	error_status = cbData->status;
//	uint8_t *pdata = (uint8_t*) &rxBuffer;
//	dwt_readrxdata(pdata, UWB_MAC_HEADER_LEN + 1, 0, pports);
	//但是为什么那个LED有正常的闪的呢？表示RXOK的？
	dwt_setrxtimeout(0, pports);
	/* Activate reception immediately. */
	dwt_rxenable(DWT_START_RX_IMMEDIATE, pports);
}


static float uwb_get_fp_angle(uint16_t fp_index, UWBPortTypeDef *pports) {
	uint8_t acc_buffer[5];
	uint8_t len = 4;
	int16_t cir_real, cir_imag;
	dwt_readaccdata(acc_buffer, len + 1, 4 * fp_index, pports);
	memcpy(&cir_real, &acc_buffer[1], sizeof(int16_t));
	memcpy(&cir_imag, &acc_buffer[3], sizeof(int16_t));
	return atan2f((float) cir_imag, (float) cir_real);
}


void anchor_parse_pdoa(UWBPortTypeDef *pports, uint16_t id, uint8_t index) {

	volatile PDoA_Struct_t* pdiag = NULL;
#if(USE_TWO_PDOA)
	for(int i = 0 ; i < ranging_num ; i++){
		if(aoa_data[i].src_car_id == id){
			pdiag = &aoa_data[i].diag[0];
			break;
		}
	}
#else
	//找到空白的
	for(int i = 0 ; i< 3; i++){
		//一定最开始使用的是
		if(pdoa_diags[i].is_used == 0 || (pdoa_diags[i].src_id == id)){
			pdiag = &pdoa_diags[i];
			pdiag->src_id = id;
			pdiag->is_used = 1;;
			break;
		}
	}
#endif
	if(pdiag == NULL) return;

	dwt_readrcphase(&(pdiag->Diag[index-1].rcphase), pports);
	dwt_readdiagnostics(&(pdiag->Diag[index-1].tempDiag), pports);
	pdiag->Diag[index-1].fp_index = (uint16_t) round(((float) (pdiag->Diag[index-1].tempDiag.firstPath & 0x3F) / 0x3F))
			+ (pdiag->Diag[index-1].tempDiag.firstPath >> 6);

	pdiag->Diag[index-1].fp_angle = uwb_get_fp_angle(pdiag->Diag[index-1].fp_index, pports);
	pdiag->Diag[index-1].avalible = 1;

//	if (htim7.State == HAL_TIM_STATE_READY)
//	//以TIM7启动与否表示是否为本轮初次收到MEASURE信息   7ms
//	{
//		__HAL_TIM_CLEAR_FLAG(&htim7, TIM_FLAG_UPDATE);
//		HAL_TIM_Base_Start(&htim7);
//	}

}

#if(USE_TWO_PDOA)
void anchor_parse_pdoa_final(UWBPortTypeDef *pports, uint16_t id, uint8_t index) {

	volatile PDoA_Struct_t* pdiag = NULL;
	for(int i = 0 ; i < ranging_num; i++){
		if(aoa_data[i].src_car_id == id){
			 pdiag = &aoa_data[i].diag[1];
			 break;
		}
	}
	if(pdiag == NULL) return;
	dwt_readrcphase(&(pdiag->Diag[index-1].rcphase), pports);
	dwt_readdiagnostics(&(pdiag->Diag[index-1].tempDiag), pports);
	pdiag->Diag[index-1].fp_index = (uint16_t) round(((float) (pdiag->Diag[index-1].tempDiag.firstPath & 0x3F) / 0x3F))
			+ (pdiag->Diag[index-1].tempDiag.firstPath >> 6);
	pdiag->Diag[index-1].fp_angle = uwb_get_fp_angle(pdiag->Diag[index-1].fp_index, pports);
	pdiag->Diag[index-1].avalible = 1;

}
#endif


void process_pdoa(uint16_t id){

	//这个到底有什么不一样的地方吗?
	UNUSED(id);
	//先关闭
	volatile AoADiagnosticTypeDef* pdw1, *pdw2;
	float temp;

	for(int i = 0; i< 3; i++){
		if(pdoa_diags[i].is_used == 1){
			if(pdoa_diags[i].Diag[0].avalible == 1 && pdoa_diags[i].Diag[1].avalible == 1){
				for(int j = 0; j < 2; j++){
					pdoa_diags[i].Diag[j].phi = pdoa_diags[i].Diag[j].fp_angle;
					pdoa_diags[i].Diag[j].beta = (float)pdoa_diags[i].Diag[j].rcphase / 64.0 * PI;
				}
				//phi1 = 0.58538 , beta1 = 5.49778
				//phi2 = -1.8843,  beta2 = 3.4852
				pdw1 = pdoa_diags[i].Diag;
				pdw2 = pdoa_diags[i].Diag + 1;
				temp = (pdw1->phi - pdw1->beta) - (pdw2->phi - pdw2->beta) + PI;
				while (temp >= PI2) {
					temp -= PI2;
				}
				while (temp < 0){
					temp += PI2;
				}
				temp -= PI;   // 所以，范围在（-PI, PI）
				if (isnormal(temp)){  //不应该不对吧？
//				if (1){
					//找到对应的节点 ~  0.07492
					for(int k = 0 ; k < MAX_TAG; k++){
						if(aoa_data[k].src_car_id == pdoa_diags[i].src_id){
							aoa_data[k].phase = temp;
							aoa_data[k].phase_m = aoa_data[k].phase * (LAMDA_M / 2.0 / PI);
							//此处进行一个限幅操作
							if(aoa_data[k].phase_m > D_M) {
								aoa_data[k].alpha = 1.0;
								aoa_data[k].theta = 90.0;
								aoa_data[k].avalible = 1;
								break;
							}
							if (aoa_data[k].phase_m < ND_M) {
								aoa_data[k].alpha = -1.0;
								aoa_data[k].theta = -90.0;
								aoa_data[k].avalible = 1;
								break;
							}
							aoa_data[k].alpha = aoa_data[k].phase_m / D_M;
							aoa_data[k].theta = asinf(aoa_data[k].alpha) / PI * 180.0;
							aoa_data[k].avalible = 1;
							break;
						}
					}
				}
			}
		}
		pdoa_diags[i].is_used = 0;
		pdoa_diags[i].src_id = 0xFFFF;
		pdoa_diags[i].Diag[0].avalible = 0;
		pdoa_diags[i].Diag[1].avalible = 0;
	}
	//重新使能接收
	enable_pdoa();

}

#if(USE_TWO_PDOA)
void process_data_diag(PDoA_Struct_t* pdiag){

	pdiag->processed = 1;
	volatile AoADiagnosticTypeDef* pdw1, *pdw2;
	float temp;
	for (int j = 0; j < 2; j++) {
		pdiag->Diag[j].phi = pdiag->Diag[j].fp_angle;
		pdiag->Diag[j].beta = (float) pdiag->Diag[j].rcphase
				/ 64.0 * PI;
	}
	pdw1 = pdiag->Diag;
	pdw2 = pdiag->Diag +1;

	temp = (pdw1->phi - pdw1->beta) - (pdw2->phi - pdw2->beta) + PI;
	while (temp >= PI2) {
		temp -= PI2;
	}
	while (temp < 0) {
		temp += PI2;
	}
	temp -= PI;   // 所以，范围在（-PI, PI）
	if (isnormal(temp)) {  //不应该不对吧？
		//				if (1){
		//找到对应的节点 ~  0.07492
		pdiag->phase  = temp;
		pdiag->phase_m = pdiag->phase * (LAMDA_M / 2.0 / PI);
		//此处进行一个限幅操作
		if (pdiag->phase_m > D_M) {
			pdiag->alpha = 1.0;
			pdiag->theta = 90.0;
			pdiag->available = 1;
		}
		if (pdiag->phase_m < ND_M) {
			pdiag->alpha = -1.0;
			pdiag->theta = -90.0;
			pdiag->available = 1;
		}
		pdiag->alpha = pdiag->phase_m / D_M;
		pdiag->theta = asinf(pdiag->alpha) / PI * 180.0;
		pdiag->available = 1;
	}
}
#endif

void rxToCallback(const dwt_cb_data_t *cbData, UWBPortTypeDef *pports) {
	/* Clear reception timeout to start next ranging process. */
	dwt_setrxtimeout(0, pports);
	/* Activate reception immediately. */
	dwt_rxenable(DWT_START_RX_IMMEDIATE, pports);
}

void txOkCallback(const dwt_cb_data_t *cbData, UWBPortTypeDef *pports) {


}





void enable_pdoa(void) {
	for (int i = 1; i < DWT_NUM_DW_DEV; i++) {
		if (UWB.ports[i].avalible == 1) {
			dwt_rxenable(DWT_START_RX_IMMEDIATE, &(UWB.ports[i]));
		}
	}
}

void disable_pdoa(void) {
	for (int i = 1; i < DWT_NUM_DW_DEV; i++) {
		if (UWB.ports[i].avalible == 1) {
			dwt_forcetrxoff(&(UWB.ports[i])); // Turn the RX off
			dwt_rxreset(&(UWB.ports[i]));
		}
	}
}




uint64_t getDeltaT(uint64_t ts1, uint64_t ts2) {
	if (ts1 > ts2) {
		return ts1 - ts2;
	} else {
		return ts1 + (0xFFFFFFFFFF - ts2);
	}
}

uint64_t getSumT(uint64_t ts1, uint64_t ts2) {
	//加，超出就减去
	uint64_t ts = ts1 + ts2;
	if (ts > 0xFFFFFFFFFF) {   //四十位的了
		ts = ts - 0xFFFFFFFFFF - 1;
	}
	return ts;
}

float uwb_calculate_rx_power(uint16_t cir_pwr, uint16_t rxpacc,
		uint16_t rxpacc_nosat) {
	float adjusted_rxpacc;
	float temp1, temp2;
	const float two_17 = (float) (1 << 17);
	if (rxpacc == rxpacc_nosat) {
		adjusted_rxpacc = rxpacc + RXPACC_ADJUSTMENT;
	} else {
		adjusted_rxpacc = rxpacc;
	}
	temp1 = (float) cir_pwr * two_17;
	temp2 = powf(adjusted_rxpacc, 2);
	temp1 = temp1 / temp2;
	temp1 = log10f(temp1);
	temp1 = 10 * temp1 - A_PRF64M;
	return temp1;
}

float uwb_calculate_fp_power(uint16_t fp_amp1, uint16_t fp_amp2,
		uint16_t fp_amp3, uint16_t rxpacc, uint16_t rxpacc_nosat) {
	float adjusted_rxpacc;
	if (rxpacc == rxpacc_nosat) {
		adjusted_rxpacc = rxpacc + RXPACC_ADJUSTMENT;
	} else {
		adjusted_rxpacc = rxpacc;
	}
	return 10
			* log10f(
					(powf((float) fp_amp1, 2) + powf((float) fp_amp2, 2)
							+ powf((float) fp_amp3, 2))
							/ powf(adjusted_rxpacc, 2)) - A_PRF64M;
}

float uwb_calculate_power(UWBPortTypeDef *pports) {
	dwt_rxdiag_t tempdiag;
	dwt_readdiagnostics(&tempdiag, pports);
	return uwb_calculate_fp_power(tempdiag.firstPathAmp1,
			tempdiag.firstPathAmp2, tempdiag.firstPathAmp3,
			tempdiag.rxPreamCount, tempdiag.rxPreamCountNOSAT);
}



//这边还需要设置相应地GPIO的各种配置，相同，然后这边的wakeup应该是高电平还是低电平的默认输出
static void initOtherPorts(void) {

	UWB.ports[1].hspi = &hspi1;
	UWB.ports[1].irq_pin = UWB1A_IRQ_Pin;
	UWB.ports[1].irq_port = UWB1A_IRQ_GPIO_Port;
	UWB.ports[1].rstn_pin = UWB1A_RSTn_Pin;
	UWB.ports[1].rstn_port = UWB1A_RSTn_GPIO_Port;
	UWB.ports[1].spi_csn_pin = UWB1A_SPICSn_Pin;
	UWB.ports[1].spi_csn_port = UWB1A_SPICSn_GPIO_Port;
	UWB.ports[1].wakeup_pin = UWB1_WAKEUP_Pin;
	UWB.ports[1].wakeup_port = UWB1_WAKEUP_GPIO_Port;
	UWB.ports[1].exti_line = EXTI0_IRQn;

	UWB.ports[2].hspi = &hspi1;
	UWB.ports[2].irq_pin = UWB1B_IRQ_Pin;
	UWB.ports[2].irq_port = UWB1B_IRQ_GPIO_Port;
	UWB.ports[2].rstn_pin = UWB1B_RSTn_Pin;
	UWB.ports[2].rstn_port = UWB1B_RSTn_GPIO_Port;
	UWB.ports[2].spi_csn_pin = UWB1B_SPICSn_Pin;
	UWB.ports[2].spi_csn_port = UWB1B_SPICSn_GPIO_Port;
	UWB.ports[2].wakeup_pin = UWB1_WAKEUP_Pin;
	UWB.ports[2].wakeup_port = UWB1_WAKEUP_GPIO_Port;
	UWB.ports[2].exti_line = EXTI1_IRQn;

//	UWB.ports[3].hspi = &hspi2;
//	UWB.ports[3].irq_pin = UWB2A_IRQ_Pin;
//	UWB.ports[3].irq_port = UWB2A_IRQ_GPIO_Port;
//	UWB.ports[3].rstn_pin = UWB2A_RSTn_Pin;
//	UWB.ports[3].rstn_port = UWB2A_RSTn_GPIO_Port;
//	UWB.ports[3].spi_csn_pin = UWB2A_SPICSn_Pin;
//	UWB.ports[3].spi_csn_port = UWB2A_SPICSn_GPIO_Port;
//	UWB.ports[3].wakeup_pin = UWB2_WAKEUP_Pin;
//	UWB.ports[3].wakeup_port = UWB2_WAKEUP_GPIO_Port;
//	UWB.ports[3].exti_line = EXTI9_5_IRQn;
//
//	UWB.ports[4].hspi = &hspi2;
//	UWB.ports[4].irq_pin = UWB2B_IRQ_Pin;
//	UWB.ports[4].irq_port = UWB2B_IRQ_GPIO_Port;
//	UWB.ports[4].rstn_pin = UWB2B_RSTn_Pin;
//	UWB.ports[4].rstn_port = UWB2B_RSTn_GPIO_Port;
//	UWB.ports[4].spi_csn_pin = UWB2B_SPICSn_Pin;
//	UWB.ports[4].spi_csn_port = UWB2B_SPICSn_GPIO_Port;
//	UWB.ports[4].wakeup_pin = UWB2_WAKEUP_Pin;
//	UWB.ports[4].wakeup_port = UWB2_WAKEUP_GPIO_Port;
//	UWB.ports[4].exti_line = EXTI9_5_IRQn;
//
//	UWB.ports[5].hspi = &hspi3;
//	UWB.ports[5].irq_pin = UWB3A_IRQ_Pin;
//	UWB.ports[5].irq_port = UWB3A_IRQ_GPIO_Port;
//	UWB.ports[5].rstn_pin = UWB3A_RSTn_Pin;
//	UWB.ports[5].rstn_port = UWB3A_RSTn_GPIO_Port;
//	UWB.ports[5].spi_csn_pin = UWB3A_SPICSn_Pin;
//	UWB.ports[5].spi_csn_port = UWB3A_SPICSn_GPIO_Port;
//	UWB.ports[5].wakeup_pin = UWB3_WAKEUP_Pin;
//	UWB.ports[5].wakeup_port = UWB3_WAKEUP_GPIO_Port;
//	UWB.ports[5].exti_line = EXTI15_10_IRQn;
//
//	UWB.ports[6].hspi = &hspi3;
//	UWB.ports[6].irq_pin = UWB3B_IRQ_Pin;
//	UWB.ports[6].irq_port = UWB3B_IRQ_GPIO_Port;
//	UWB.ports[6].rstn_pin = UWB3B_RSTn_Pin;
//	UWB.ports[6].rstn_port = UWB3B_RSTn_GPIO_Port;
//	UWB.ports[6].spi_csn_pin = UWB3B_SPICSn_Pin;
//	UWB.ports[6].spi_csn_port = UWB3B_SPICSn_GPIO_Port;
//	UWB.ports[6].wakeup_pin = UWB3_WAKEUP_Pin;
//	UWB.ports[6].wakeup_port = UWB3_WAKEUP_GPIO_Port;
//	UWB.ports[6].exti_line = EXTI15_10_IRQn;
//
//	UWB.ports[7].hspi = &hspi4;
//	UWB.ports[7].irq_pin = UWB4A_IRQ_Pin;
//	UWB.ports[7].irq_port = UWB4A_IRQ_GPIO_Port;
//	UWB.ports[7].rstn_pin = UWB4A_RSTn_Pin;
//	UWB.ports[7].rstn_port = UWB4A_RSTn_GPIO_Port;
//	UWB.ports[7].spi_csn_pin = UWB4A_SPICSn_Pin;
//	UWB.ports[7].spi_csn_port = UWB4A_SPICSn_GPIO_Port;
//	UWB.ports[7].wakeup_pin = UWB4_WAKEUP_Pin;
//	UWB.ports[7].wakeup_port = UWB4_WAKEUP_GPIO_Port;
//	UWB.ports[7].exti_line = EXTI3_IRQn;
//
//	UWB.ports[8].hspi = &hspi4;
//	UWB.ports[8].irq_pin = UWB4B_IRQ_Pin;
//	UWB.ports[8].irq_port = UWB4B_IRQ_GPIO_Port;
//	UWB.ports[8].rstn_pin = UWB4B_RSTn_Pin;
//	UWB.ports[8].rstn_port = UWB4B_RSTn_GPIO_Port;
//	UWB.ports[8].spi_csn_pin = UWB4B_SPICSn_Pin;
//	UWB.ports[8].spi_csn_port = UWB4B_SPICSn_GPIO_Port;
//	UWB.ports[8].wakeup_pin = UWB4_WAKEUP_Pin;
//	UWB.ports[8].wakeup_port = UWB4_WAKEUP_GPIO_Port;
//	UWB.ports[8].exti_line = EXTI4_IRQn;

}

