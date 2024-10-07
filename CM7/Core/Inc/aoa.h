# ifndef __AOA_H__
# define __AOA_H__

#include "main.h"


#define UWB_CARRIER_FREQ_CH2 		(4.0e9f)
#define LAMDA_M 					(SPEED_OF_LIGHT / UWB_CARRIER_FREQ_CH2)	//wave length of carrier wave
#define D_M							(0.02f)   //distance between two antennas

typedef struct
{
	float phi;
	float beta;
	uint16_t src_car_id;
	uint8_t avalible;
} AoAParamTypeDef;





# endif
