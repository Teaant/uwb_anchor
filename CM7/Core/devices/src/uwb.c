#include "uwb.h"
#include "corecomm.h"
#include "main.h"
# include "agent.h"
#include "spi.h"
#include <math.h>
#include <uwb_mac.h>

#include "kalman_filter.h"

#include "task_manager.h"

#include "../../../util/msg_queue.h"


uint8_t test_pdoa = 1;

volatile uint8_t err_times = 0;
volatile uint8_t rx_times = 0;

//#define ACC_MEM_TEST 1
/* Default communication configuration. We use here EVK1000's default mode (mode 3). */

#define MIN_POWER	1
//但是85是为?
__attribute__((aligned(4))) Flash_Config_t flash_config = {
		.tx_power = DEFAULT_TX_POWER,
		.tcp_server = 2,
		.tag_interval = INTERVAL,

};
/**
 */
static dwt_config_t config = {
    2,               /* Channel number. */
    DWT_PRF_64M,     /* Pulse repetition frequency. */
    DWT_PLEN_1024,   /* Preamble length. Used in TX only. */
    DWT_PAC32,       /* Preamble acquisition chunk size. Used in RX only. */
    9,               /* TX preamble code. Used in TX only. */
    9,               /* RX preamble code. Used in RX only. */
    0,               /* 0 to use standard SFD, 1 to use non-standard SFD. */
    DWT_BR_850K,     /* Data rate. */
    DWT_PHRMODE_STD, /* PHY header mode. */
    (1025 + 64 - 32) /* SFD timeout (preamble length + 1 + SFD length - PAC size). Used in RX only. */
};

/* Declaration of static functions. */


UWBDef UWB={
		.err_time = 0,
		.antDelay = RX_ANT_DLY
};


extern UWB_Node_t uwb_node;
extern TxDoneCb txDoneCallbacks[];

extern UART_HandleTypeDef huart2;

extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim7;

extern SPI_HandleTypeDef hspi6;

static uint16_t TX_Buffers[4] = {TX_BUFFER_1, TX_BUFFER_2, TX_BUFFER_3, TX_BUFFER_4};


#define		MAX_POWER		1


int32_t uwbInit(uint16_t ID)
{

	////min : C0   max:1F	 //with 85 default and nothing special ~
#if(MIN_POWER)
	dwt_txconfig_t config_t = {
			.PGdly = 0xC2,
			.power = flash_config.tx_power}; //1110 0000
#else
	dwt_txconfig_t config_t = {
				.PGdly = 0xC2,
				.power = 0x851F1F85
		};   //1110 0000
#endif


    UWB.ports[0].hspi = &hspi6;
    UWB.ports[0].irq_pin = UWB_IRQn_Pin;
    UWB.ports[0].irq_port = UWB_IRQn_GPIO_Port;
    UWB.ports[0].rstn_pin = UWB_RSTn_Pin;
    UWB.ports[0].rstn_port = UWB_RSTn_GPIO_Port;
    UWB.ports[0].spi_csn_pin = UWB_SPICSn_Pin;
    UWB.ports[0].spi_csn_port = UWB_SPICSn_GPIO_Port;
    UWB.ports[0].wakeup_pin = 0;
    UWB.ports[0].wakeup_port = NULL;
    UWB.ports[0].exti_line = EXTI15_10_IRQn;
    UWB.ports[0].port_type = Ds_Twr_Port;
    UWB.ports[0].port_init = MX_SPI6_Init;
    //初始化操作

//	initOtherPorts();

    for(int i = 0;i<DWT_NUM_DW_DEV;i++)
    {
    	reset_DW1000(&UWB.ports[i]);
    	UWB.ports[i].avalible = 0;

    }
    for(int i = 0;i < DWT_NUM_DW_DEV;i++)
    {
    	UWBPortTypeDef *pports = &UWB.ports[i];
    	int trys = 10;
    	port_set_dw1000_slowrate(pports);
    	dwt_setlocaldataptr(i);
    	do{
			if (dwt_initialise(DWT_LOADUCODE, pports) == DWT_SUCCESS)
			{
				UWB.ports[i].avalible = 1;
			}
			else
			{
				trys--;
			}
    	} while((trys > 0) && UWB.ports[i].avalible != 1);
    	if(trys <= 0)
    	{
    		continue;
    	}

//		SYS_CFG_FF_ALL_EN

    	/**
    	 * dwt_enableframefilter()
    	 *  * @brief This is used to enable the frame filtering - (the default option is to
    	 * accept any data and ACK frames with correct destination address
    	 */

    	port_set_dw1000_fastrate(pports);

		dwt_setpanid(PAN_ID, pports);
		dwt_setaddress16(ID, pports);
		dwt_enableframefilter(DWT_FF_DATA_EN | DWT_FF_RSVD_EN , pports);

		/*Setup Interrupt*/
		dwt_setinterrupt((SYS_STATUS_TXFRS|SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_ERR | SYS_STATUS_ALL_RX_TO), 2, pports);
//		dwt_setcallbacks(NULL, rxOkCallback_Ranging, rxToCallback, rxErrCallback);
		if(i == 0){
			dwt_setcallbacks(txOkCallback, rxOkCallback_Ranging, rxToCallback, rxErrCallback);  //不同的使用不同的这个callback呢？
		}
//		else{
//			dwt_setcallbacks(NULL, rxOkCallback_PDoA, rxToCallback, rxErrCallback);  //不同的使用不同的这个callback呢？
//		}

		port_set_deca_isr(dwt_isr);
		dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_GOOD|SYS_STATUS_ALL_RX_TO|SYS_STATUS_ALL_RX_ERR, pports);
		/* Configure DW1000. See NOTE 7 below. */
		dwt_configure(&config, pports);

#if(MAX_POWER)
		//2024.06.03  发射功测试
		dwt_setsmarttxpower(0, pports);
		//设置发射功率
		dwt_configuretxrf(&config_t, pports);
		//end
#endif
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

    uwb_node.device = &UWB;
    return 0;
}



uint64 get_tx_timestamp_u64(UWBPortTypeDef *pports)
{
    uint8 ts_tab[5];
    uint64 ts = 0;
    int i;
    dwt_readtxtimestamp(ts_tab, pports);
    for (i = 4; i >= 0; i--)
    {
        ts <<= 8;
        ts |= ts_tab[i];
    }
    return ts;
}

uint64_t get_sys_timestamp(UWBPortTypeDef *pports) {
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
uint64 get_rx_timestamp_u64(UWBPortTypeDef *pports)
{
    uint8 ts_tab[5];
    uint64 ts = 0;
    int i;
    dwt_readrxtimestamp(ts_tab, pports);
    for (i = 4; i >= 0; i--)
    {
        ts <<= 8;
        ts |= ts_tab[i];
    }
    return ts;
}

void rxOkCallback_Ranging(const dwt_cb_data_t *cbData, UWBPortTypeDef *pports){

#if(MY_ROLE == ANCHOR)
	UWB_Rcv_Msg_t* pmsg = req_msg_buffer();
	if(pmsg != NULL){
		uint8_t * pdata = (uint8_t*)pmsg->rcv_data;
		uint8_t msg_len = (cbData->datalength <= MAX_MSG_READ_LEN )?(cbData->datalength) : (MAX_MSG_READ_LEN);

		pmsg->rx_ts = get_rx_timestamp_u64(&(UWB.ports[0]));

		dwt_readrxdata(pdata, msg_len, 0,pports);
		if(put_msg_in_queue(pmsg)){
			enqueueTask(anchor_parse_ranging, 0);
		}
	}
#if(USE_TIMER)
	dwt_rxenable(DWT_START_RX_IMMEDIATE, pports);
#endif
#else
	rx_times ++;
	uint64_t rx_ts = get_rx_timestamp_u64(&(UWB.ports[0]));
	volatile uint8_t * pdata = uwb_node.ptag_struct->rxBuffer;
	dwt_readrxdata(pdata, cbData->datalength, 0 , pports);
	tag_parse_ranging(rx_ts);

#endif

}

//void rxOkCallback_PDoA(const dwt_cb_data_t*, UWBPortTypeDef *pports) {
//
//	//有这个frame_control的类
//	uint8_t pdoa_id = pports - UWB.ports;
//
//	uint8_t* pdata = (uint8_t*)&rxBuffer;
//
//	dwt_readrxdata(pdata, UWB_MAC_HEADER_LEN + 1, 0, pports);
//
////	HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
//
//	uint8_t frame_type = GET_FRAMETYPE(rxBuffer.header.control);
////	if(rxBuffer.function == UWB_Ranging_Resp)
//
//	if (frame_type != data_frame || rxBuffer.function != UWB_Ranging_Poll){
//		/* Activate reception immediately. */
//		dwt_rxenable(DWT_START_RX_IMMEDIATE , pports);
//		return;
//	}
//
//	anchor_parse_pdoa(pports, rxBuffer.header.src, pdoa_id);
//
//	dwt_rxenable(DWT_START_RX_IMMEDIATE , pports);
//	//也不知这段时间需要多少？
//
////	uint64_t time = get_sys_timestamp(pports);
//
//}



void set_rx_timeout(uint16_t timeout){

	dwt_setrxtimeout(timeout, &UWB.ports[0]);

}

void disable_rx_timeout(void){
	dwt_setrxtimeout(0, &UWB.ports[0]);
}

void enable_rx_with_timeout(uint16_t timeout){

	dwt_setrxtimeout(timeout, &UWB.ports[0]);
	dwt_rxenable(DWT_START_RX_IMMEDIATE, &UWB.ports[0]);

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

void rxToCallback(const dwt_cb_data_t *cbData, UWBPortTypeDef *pports)
{
#if(USE_TIMER)
	/* Clear reception timeout to start next ranging process. */
    dwt_setrxtimeout(0, pports);
    /* Activate reception immediately. */
    dwt_rxenable(DWT_START_RX_IMMEDIATE, pports);
	//要另外弄了哈
	//beacon 是不需要的
#else
#if(MY_ROLE == ANCHOR)
	if(uwb_node.state == ranging){
		//先看一下吧看行不行
	}else{
		dwt_setrxtimeout(0, pports);
		dwt_rxenable(DWT_START_RX_IMMEDIATE, pports);
	}
#else
	dwt_setrxtimeout(0, pports);
	dwt_rxenable(DWT_START_RX_IMMEDIATE, pports);
#endif
#endif
}

void txOkCallback(const dwt_cb_data_t *cbData, UWBPortTypeDef *pports){

	if(txDoneCallbacks[uwb_node.state] != NULL){
		uint64_t tx_ts = get_tx_timestamp_u64(&(UWB.ports[0]));
		txDoneCallbacks[uwb_node.state](tx_ts);
	}

}

void rxErrCallback(const dwt_cb_data_t *cbData, UWBPortTypeDef *pports)
{

	//其实这边如果出现什么接受错误，应该按照当前的状态做一些改动的 ~
	/* Clear reception timeout to start next ranging process. */
	HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);

    dwt_setrxtimeout(0, pports);
    /* Activate reception immediately. */
    dwt_rxenable(DWT_START_RX_IMMEDIATE, pports);
}

void enable_ranging(void){
	dwt_rxenable(DWT_START_RX_IMMEDIATE, &(UWB.ports[0]));
}

//void enable_pdoa(void){
//	for(int i = 1; i< DWT_NUM_DW_DEV; i++){
//
//		if(UWB.ports[i].avalible == 1){
//#if(DEBUG)
//		printf("available %d\r\n", i);
//#endif
//		dwt_rxenable(DWT_START_RX_IMMEDIATE, &(UWB.ports[i]));
//		}
//	}
//}

//void disable_pdoa(void) {
//	for (int i = 1; i < DWT_NUM_DW_DEV; i++) {
//		if (UWB.ports[i].avalible == 1) {
//			dwt_forcetrxoff(&(UWB.ports[i])); // Turn the RX off
//			dwt_rxreset(&(UWB.ports[i]));
//		}
//	}
//}


//void anchor_parse_pdoa(UWBPortTypeDef *pports) {
//
//	uint64_t rx_ts_64 = (uint64_t) get_rx_timestamp_u64(pports);
//	uint32_t rx_ts = (uint32_t) rx_ts_64;
//	uint16_t src_car_id = uwb_node.pdoa_buffer.header.src;
//	uint8_t my_dw_id = pports - &UWB.ports[0];
//	uint8_t rcphase;
//	dwt_rxdiag_t tempdiag;
//	uint8_t tempacc[129];
//	uint16_t fp_index;
//	float fp_angle;
//	AoADiagnosticTypeDef *pdiag = &aoa_diagnostic[my_dw_id];
//
//	dwt_readrcphase(&rcphase, pports);
//	dwt_readdiagnostics(&tempdiag, pports);
//	fp_index = (uint16_t) round(((float) (tempdiag.firstPath & 0x3F) / 0x3F))
//			+ (tempdiag.firstPath >> 6);
//	fp_angle = uwb_get_fp_angle(fp_index, pports);
//
//	pdiag->avalible = 1;
//	pdiag->fp_amp1 = tempdiag.firstPathAmp1;
//	pdiag->fp_amp2 = tempdiag.firstPathAmp2;
//	pdiag->fp_amp3 = tempdiag.firstPathAmp3;
//	pdiag->fp_amp_sum = pdiag->fp_amp1 + pdiag->fp_amp2 + pdiag->fp_amp3;
//	pdiag->fp_angle = fp_angle;
//	pdiag->fp_index = fp_index;
//	pdiag->my_dw_id = my_dw_id;
//	pdiag->rcphase = rcphase;
//	pdiag->rx_ts = rx_ts;
//	pdiag->std_noise = tempdiag.stdNoise;
//	pdiag->cir_pwr = tempdiag.maxGrowthCIR;
//	pdiag->rxpacc = tempdiag.rxPreamCount;
//	pdiag->rxpacc_nosat = tempdiag.rxPreamCountNOSAT;
//
//	//test
//	if (pdiag->fp_amp1 < (uint16_t) 256) {
//		pdiag->avalible = 0;
//		return;
//	}
//
//
//	UWB.aoa_param[my_dw_id].phi = fp_angle;
//	UWB.aoa_param[my_dw_id].beta = (float) rcphase / 64.0 * PI;
//	UWB.aoa_param[my_dw_id].avalible = 1;
//	UWB.aoa_param[my_dw_id].src_car_id = src_car_id;
//
//	if (htim7.State == HAL_TIM_STATE_READY)
//	//以TIM7启动与否表示是否为本轮初次收到MEASURE信息   5ms
//	{
//		HAL_TIM_Base_Start(&htim7);			//启动TIM7
//		__HAL_TIM_ENABLE_IT(&htim7, TIM_IT_UPDATE);
//		HAL_NVIC_EnableIRQ(TIM7_IRQn);
//	}
//	/* Clear reception timeout to start next ranging process. */
////	dwt_setrxtimeout(0, pports);
//	/* Activate reception immediately. */
//
//}

//(1) if have to set the delay
//final_tx_time = (resp_rx_ts + (RESP_RX_TO_FINAL_TX_DLY_UUS * UUS_TO_DWT_TIME)) >> 8;


//不考虑延迟发送的，只是向缓冲区写数据   起始地址0
void UWB_Write_Tx_Buffer(uint8_t* pdata, uint8_t len){

	UWBPortTypeDef *pports = &(UWB.ports[0]);
	dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS, pports);
	dwt_writetxdata(len, pdata, 0, pports); /* Zero offset in TX buffer. */

}

void UWB_Schedule_Beacon_Frame(uint8_t *pdata, uint8_t len, uint64_t tx_time, uint8_t is_timeout, uint8_t bop) {

	UWBPortTypeDef *pports = &(UWB.ports[0]);

	uint32_t delayed_time = (uint32_t)(tx_time >> 8);
	dwt_setdelayedtrxtime(delayed_time, pports);

	dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS, pports);

	dwt_writetxdata(len, pdata, 0, pports); /* Zero offset in TX buffer. */
	dwt_writetxfctrl(len, 0, 1, pports); /* Zero offset in TX buffer, ranging. */

	/* Start transmission, indicating that a response is expected so that reception is enabled automatically after the frame is sent and the delay
	 * set by dwt_setrxaftertxdelay() has elapsed. */
#if(!USE_TIMER)
	if(is_timeout != 0){
		if(bop == 0 || bop == 2){
			dwt_setrxtimeout(RX_TIMEOUT_10MS, pports);
		}else{
			dwt_setrxtimeout(RX_TIMEOUT_8MS, pports);
		}
	}else{
		dwt_setrxtimeout(0, pports);
	}
#endif
//	dwt_setrxaftertxdelay(RX_ENABLE_DELAYED_MS,  pports);  //设置1ms吧？
	dwt_starttx(DWT_START_TX_DELAYED | DWT_RESPONSE_EXPECTED, pports);

}

void UWB_Issue_Beacon_Frame(uint8_t *pdata, uint8_t len) {

	UWBPortTypeDef *pports = &(UWB.ports[0]);

	dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS, pports);

	dwt_writetxdata(len, pdata, 0, pports); /* Zero offset in TX buffer. */
	dwt_writetxfctrl(len, 0, 1, pports); /* Zero offset in TX buffer, ranging. */

	dwt_setrxtimeout(0, pports); //set timeout ~
//	dwt_setrxaftertxdelay(RX_ENABLE_DELAYED_MS,  pports);
	dwt_starttx(DWT_START_TX_IMMEDIATE | DWT_RESPONSE_EXPECTED, pports);

}



//可以只写ID部分可能还可以~
void UWB_Configure_Resp_In_Buffer(uint8_t* pdata, uint8_t len, uint8_t addr, uint64_t tx_time){

	UWBPortTypeDef *pports = &(UWB.ports[0]);
	dwt_writetxdata(len, pdata, TX_Buffers[addr], pports); /* Non-Zero offset in TX buffer. */
	dwt_writetxfctrl(len, TX_Buffers[addr], 1, pports);  //只改后面的什么呗？
#if(!USE_TIMER)
	uint32_t delayed_time =(uint32_t)(tx_time >> 8);   //取高八位   //暂时不采用延迟发送 ~
	dwt_setdelayedtrxtime(delayed_time, pports);
	//持续保持接受  //不知是否可以这边 ~
	dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS, pports);
	dwt_setrxtimeout(0, pports); //set no timeout ~
	dwt_starttx(DWT_START_TX_DELAYED | DWT_RESPONSE_EXPECTED, pports);
#endif

}



void issue_resp(void) {

	UWBPortTypeDef *pports = &(UWB.ports[0]);

	dwt_forcetrxoff(pports); // Turn the RX off
//	dwt_rxreset(pports);

	dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS, pports);

	dwt_setrxaftertxdelay(RX_ENABLE_DELAYED_MS * 2,  pports);
	dwt_starttx(DWT_START_TX_IMMEDIATE|DWT_RESPONSE_EXPECTED, pports);

}

void UWB_Anchor_Ack_Req(uint8_t* pdata, uint8_t len, uint64_t tx_time){

	UWBPortTypeDef *pports = &(UWB.ports[0]);

	dwt_forcetrxoff(pports); // Turn the RX off
	dwt_rxreset(pports);
	uint32_t delayed_time =(uint32_t)(tx_time >> 8);   //取高八位   //暂时不采用延迟发送 ~
	dwt_setdelayedtrxtime(delayed_time, pports);

	dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS, pports);

	dwt_writetxdata(len, pdata, TX_Buffers[2], pports); /* Zero offset in TX buffer. */
	dwt_writetxfctrl(len, TX_Buffers[2], 0, pports); /* Zero offset in TX buffer, ranging. */

//	dwt_setrxaftertxdelay(RX_ENABLE_DELAYED_MS * 2,  pports);
	dwt_starttx(DWT_START_TX_DELAYED | DWT_RESPONSE_EXPECTED, pports);

}

void UWB_Tag_Send_Ranging(uint8_t* pdata, uint8_t len, uint64_t tx_time){

	UWBPortTypeDef *pports = &(UWB.ports[0]);

	dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS, pports);

	dwt_writetxdata(len, pdata, TX_Buffers[0], pports); /* Zero offset in TX buffer. */
	dwt_writetxfctrl(len, TX_Buffers[0], 0, pports); /* Zero offset in TX buffer, ranging. */
	uint32_t delayed_time = (uint32_t)(tx_time>>8);
	dwt_setdelayedtrxtime(delayed_time, pports);

//	dwt_setrxaftertxdelay(RX_ENABLE_DELAYED_MS,  pports);	//也可以，节省一点时间需要吗？

	dwt_starttx(DWT_START_TX_DELAYED, pports);

}

void UWB_Tag_Req_Join(uint8_t* pdata, uint8_t len, uint64_t tx_time){

	UWBPortTypeDef *pports = &(UWB.ports[0]);

	uint32_t delayed_time = (uint32_t)(tx_time>>8);
	dwt_setdelayedtrxtime(delayed_time, pports);

	dwt_writetxdata(len, pdata, TX_Buffers[2], pports); /* Zero offset in TX buffer. */
	dwt_writetxfctrl(len, TX_Buffers[2], 0, pports); /* Zero offset in TX buffer, ranging. */

	dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS, pports);
	dwt_setrxaftertxdelay(0,  pports);

	dwt_starttx(DWT_START_TX_DELAYED | DWT_RESPONSE_EXPECTED, pports);  //为啥不处接收状态？ 什么个事儿？

}

//Really issue the TX
void UWB_StartTx(uint8_t is_expect){

	UWBPortTypeDef *pports = &(UWB.ports[0]);

	dwt_forcetrxoff(pports); //Turn the RX off
	dwt_rxreset(pports);

	dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS, pports);
	if(is_expect == 1){
		dwt_starttx(DWT_START_TX_IMMEDIATE | DWT_RESPONSE_EXPECTED, pports);
	}else{
		dwt_starttx(DWT_START_TX_IMMEDIATE , pports);
	}
}




//发送消息 (2)
void UWB_Send(uint8_t * pdata, uint8_t len, If_Delay_t is_delayed, uint32_t tx_time, If_Expected_t is_expect)
{
	UWBPortTypeDef *pports = &(UWB.ports[0]);

	dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS, pports);
	if(is_delayed){  //延迟发送
		dwt_setdelayedtrxtime(tx_time, pports);
	}
    dwt_forcetrxoff(pports); // Turn the RX off
    dwt_rxreset(pports);
    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS, pports);

	dwt_writetxdata(len, pdata, 0, pports); /* Zero offset in TX buffer. */
	dwt_writetxfctrl(len, 0, 1, pports); /* Zero offset in TX buffer, ranging. */

	/* Start transmission, indicating that a response is expected so that reception is enabled automatically after the frame is sent and the delay
	 * set by dwt_setrxaftertxdelay() has elapsed. */
	if((!is_delayed)&&is_expect){
		dwt_starttx(DWT_START_TX_IMMEDIATE | DWT_RESPONSE_EXPECTED, pports);
	}else if(is_delayed && is_expect){
		dwt_starttx(DWT_START_TX_DELAYED | DWT_RESPONSE_EXPECTED , pports);
	}else if(is_delayed &&(!is_expect)){
		dwt_starttx(DWT_START_TX_DELAYED , pports);
	}else{
		dwt_starttx(DWT_START_TX_IMMEDIATE , pports);
	}
}

uint64_t getDeltaT(uint64_t ts1, uint64_t ts2)
{
	if(ts1 > ts2)
	{
		return ts1-ts2;
	}
	else
	{
		return ts1+(0xFFFFFFFFFF-ts2);
	}
}

uint64_t getSumT(uint64_t ts1, uint64_t ts2){
	//加，超出就减去
	uint64_t ts = ts1+ts2;
	if(ts > 0xFFFFFFFFFF){   //四十位的了
		ts = ts - 0xFFFFFFFFFF - 1;
	}
	return ts;
}


float uwb_calculate_rx_power(uint16_t cir_pwr, uint16_t rxpacc, uint16_t rxpacc_nosat){
	float adjusted_rxpacc;
	float temp1, temp2;
	const float two_17 = (float)(1<<17);
	if(rxpacc == rxpacc_nosat){
		adjusted_rxpacc = rxpacc + RXPACC_ADJUSTMENT;
	}
	else{
		adjusted_rxpacc = rxpacc;
	}
	temp1 = (float)cir_pwr * two_17;
	temp2 = powf(adjusted_rxpacc, 2);
	temp1 = temp1 / temp2;
	temp1 = log10f(temp1);
	temp1 = 10 * temp1 - A_PRF64M;
	return temp1;
}

float uwb_calculate_fp_power(uint16_t fp_amp1, uint16_t fp_amp2, uint16_t fp_amp3, uint16_t rxpacc, uint16_t rxpacc_nosat){
	float adjusted_rxpacc;
	if(rxpacc == rxpacc_nosat){
		adjusted_rxpacc = rxpacc + RXPACC_ADJUSTMENT;
	}
	else{
		adjusted_rxpacc = rxpacc;
	}
	return 10*log10f((powf((float)fp_amp1,2) + powf((float)fp_amp2,2) + powf((float)fp_amp3,2)) / powf(adjusted_rxpacc,2)) - A_PRF64M;
}

float uwb_calculate_power(UWBPortTypeDef *pports){
	dwt_rxdiag_t tempdiag;
	dwt_readdiagnostics(&tempdiag, pports);
	return uwb_calculate_fp_power(tempdiag.firstPathAmp1, tempdiag.firstPathAmp2, tempdiag.firstPathAmp3, tempdiag.rxPreamCount, tempdiag.rxPreamCountNOSAT);
}


float array_average(float* array, uint8_t num)
{
	float *p = array;
	float sum = 0;
	for(uint8_t i = 0; i < num;i++ )
	{
		sum += *p;
		p++;
	}
	return sum/(float)num;
}

//补偿？
float distance_compensate(float distance)
{
	if(distance > 1)
		return distance;
	else
		return (-0.1966*distance*distance+1.4134*distance-0.2207);
}


void sliding_filter(float *distance)
{
	*distance = (0.7*(*distance)+0.2*(*(distance+1)) + 0.1*(*(distance+2)));
}


