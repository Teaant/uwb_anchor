/*
 * config.h
 *
 *  Created on: Jan 17, 2021
 *      Author: 12939 朱冉
 */

#ifndef INC_CONFIG_H_
#define INC_CONFIG_H_



//该宏选择小车电机类型，目前有减速电机(GEAR_MOTOR)和步进电机两种(STEP_MOTOR)
#define MOTOR_TYPE 		1


#define GEAR_MOTOR			1
#define STEP_MOTOR			2




//# define WHEEL_TRACK_LR 0.18
//小车轮子直径,单位m
# if(MOTOR_TYPE == GEAR_MOTOR)

# define WHEEL_DIAMETER 0.06685
//小车左右轮间距,单位m
# define WHEEL_TRACK_LR 0.2085

# elif(MOTOR_TYPE == STEP_MOTOR)
# define WHEEL_DIAMETER 0.066
//小车左右轮间距,单位m
# define WHEEL_TRACK_LR 0.1867
# endif


# define PI 3.1415926

//该宏选择小车上IMU传感器，目前有LSM9DS1和BMF055两种
# define IMU_SENSOR 		ICM20948

# define LSM9DS1			1
# define BMF055				2
# define ICM20948			3

# define FUTION_LIBRARY 	XIO_FUTION
# define MOTIONFX			1
# define XIO_FUTION			2


#endif /* INC_CONFIG_H_ */
