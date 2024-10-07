# ifndef __AOA_H__
# define __AOA_H__

#include "main.h"

typedef struct
{
	float phi;
	float beta;
	uint16_t src_car_id;
	uint8_t avalible;
} AoAParamTypeDef;



#define AOA_ANGLE_TOLERANCE 180
#define AOA_CALIBRATION_TABLE_LENGTH 	9

typedef struct {
	float last_valid_aoa;
	uint8_t counts_after_last_aoa;
	float last_heading;
} AOADataFilterTypeDef;

#endif


//typedef struct{
//	float measure_angle[AOA_CALIBRATION_TABLE_LENGTH];
//	float real_angle[AOA_CALIBRATION_TABLE_LENGTH];
//} AoACalibrationTypeDef;

