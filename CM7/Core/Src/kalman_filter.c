# include "kalman_filter.h"
# include "main.h"
# include "math.h"
kalman2_state state;
float init_x[2][1] = {{0},{0}};
float init_p[2][2] = {{1,0},{0,1}};


//记录连续超限次数
uint8_t tolerance_count = 0;

float predict_coordinate[2][1] = {0};

void kalman2_init(kalman2_state *state, float (*init_x)[1], float (*init_p)[2])
{
	memcpy(&state->x[0][0], &init_x[0][0], sizeof(state->x));
	memcpy(&state->p[0][0], &init_p[0][0], sizeof(state->p));
	//状态矩阵
	state->A[0][0] = 1;
	state->A[0][1] = 0;
	state->A[1][0] = 0;
	state->A[1][1] = 1;

	//输入矩阵
	state->B[0][0] = 1;
	state->B[0][1] = 0;
	state->B[1][0] = 0;
	state->B[1][1] = 1;
	//输出矩阵
	state->H[0][0] = 1;
	state->H[0][1] = 0;
	state->H[1][0] = 0;
	state->H[1][1] = 1;
//	//过程噪声的协方差矩阵，应是对角矩阵
//	state->q[0][0] = Q_VALUE_INIT;
//	state->q[0][1] = 0;
//	state->q[1][0] = 0;
//	state->q[1][1] = Q_VALUE_INIT;
//
//	//测量噪声的协方差矩阵，应是对角矩阵
//	state->r[0][0] = R_VALUE_INIT;
//	state->r[0][1] = 0;
//	state->r[1][0] = 0;
//	state->r[1][1] = R_VALUE_INIT;
//
//	state->mode = KalmanMode_Init;


//	过程噪声的协方差矩阵，应是对角矩阵
	state->q[0][0] = Q_VALUE_RUN;
	state->q[0][1] = 0;
	state->q[1][0] = 0;
	state->q[1][1] = Q_VALUE_RUN;

	//测量噪声的协方差矩阵，应是对角矩阵
	state->r[0][0] = R_VALUE_RUN;
	state->r[0][1] = 0;
	state->r[1][0] = 0;
	state->r[1][1] = R_VALUE_RUN;

	state->mode = KalmanMode_Run;

	state->timestamp = 0;
}


void kalman2_filter(kalman2_state *state, float (*u)[1], float (*z_measure)[1])
{
	float temp1 = 0;
	float temp2 = 0;

	if(state->timestamp < INIT_TIME)
	{
		state->timestamp ++;
		state->x[0][0] = z_measure[0][0];
		state->x[1][0] = z_measure[1][0];
		return;
	}

	//1、Prediction：预测
	state->x[0][0] = state->A[0][0]*state->x[0][0] + state->A[0][1]*state->x[1][0] + \
					state->B[0][0]*u[0][0] + state->B[0][1]*u[1][0];

	state->x[1][0] = state->A[1][0]*state->x[0][0] + state->A[1][1]*state->x[1][0] + \
					state->B[1][0]*u[0][0] + state->B[1][1]*u[1][0];

	predict_coordinate[0][0] = state->x[0][0];
	predict_coordinate[1][0] = state->x[1][0];
	//因为我们的系统A为单位阵，对于误差协方差矩阵的计算被简化了，如果A不是单位阵，需要根据公式p(k) = A*p(k-1)*A_t+q
	state->p[0][0] += state->q[0][0];
	state->p[0][1] += state->q[0][1];
	state->p[1][0] += state->q[1][0];
	state->p[1][1] += state->q[1][1];


	//2、Measure:测量

	//K = p*H_t*inv(H*p*H_t+R)
	//在这里我们的H也是单位矩阵，可以简化计算如下：K=p*inv(p+R)
	//这几个还都是对角矩阵，太好算了！
	temp1 = state->p[0][0] + state->r[0][0];
	temp1 = 1/temp1;
	temp2 = state->p[1][1] + state->r[1][1];
	temp2 = 1/temp2;

	state->gain[0][0] = state->p[0][0] * temp1;
	state->gain[1][1] = state->p[1][1] * temp2;
	state->gain[0][1] = 0;
	state->gain[1][0] = 0;

	//x^ = x^ + K(y-Cx^)
	state->x[0][0] = state->x[0][0] + state->gain[0][0]*(z_measure[0][0]-state->x[0][0]);
	state->x[1][0] = state->x[1][0] + state->gain[1][1]*(z_measure[1][0]-state->x[1][0]);

	//p=(I-K*H)*p
	state->p[0][0] = (1-state->gain[0][0])*state->p[0][0];
	state->p[1][1] = (1-state->gain[1][1])*state->p[1][1];



	state->measured_x[0][0] = z_measure[0][0];
	state->measured_x[1][0] = z_measure[1][0];
//	mode_switch(&state, TOLERANCE,TOLERANCE_COUNT_MAX,&tolerance_count);

	}


void mode_switch(kalman2_state *state,float tolerance,uint8_t tolerance_count_max, uint8_t* p_count)
{
	if(state->mode == KalmanMode_Init)
	{
		//init模式下，误差连续小于容忍度多次后切换系统
		if(sqrt(pow(state->x[0][0]-state->measured_x[0][0],2) + pow(state->x[1][0]-state->measured_x[1][0],2)) <= tolerance)
		{
			*p_count++;
			if(*p_count > tolerance_count_max)
			{
				state->mode = KalmanMode_Run;
				state->q[0][0] = Q_VALUE_RUN;
				state->q[1][1] = Q_VALUE_RUN;
				state->r[0][0] = R_VALUE_RUN;
				state->r[1][1] = R_VALUE_RUN;
				*p_count = 0;
			}
		}
		else
		{
			*p_count = 0;
		}
	}
//	else if(state->mode == KalmanMode_Run)
//	{
//		//run模式下，误差连续大于容忍度多次后切换系统
//		if(sqrt(pow(state->x[0][0]-state->measured_x[0][0],2) + pow(state->x[1][0]-state->measured_x[1][0],2)) <= tolerance)
//		{
//			*p_count++;
//			if(*p_count > tolerance_count_max)
//			{
//				state->mode = KalmanMode_Init;
//				state->q[0][0] = Q_VALUE_INIT;
//				state->q[1][1] = Q_VALUE_INIT;
//				state->r[0][0] = R_VALUE_INIT;
//				state->r[1][1] = R_VALUE_INIT;
//				*p_count = 0;
//			}
//		}
//		else
//		{
//			*p_count = 0;
//		}
//	}

	}


