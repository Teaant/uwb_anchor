# ifndef __KALMAN_FILTER_H__
# define __KALMAN_FILTER_H__

# include "main.h"
//过程噪声的协方差
//# define Q 0.00001f
////测量噪声的协方差
//# define R 0.3f

# define INIT_TIME 100

//信任测量值，快速收敛
# define Q_VALUE_INIT 0.1
# define R_VALUE_INIT 0.001

//信任预测值，准确平滑
//0.0001和0.1很平滑，但收敛很慢（0.5m误差可能要几分钟）
//0.0001和0.05很平滑，但收敛很慢（0.5m误差可能要几分钟）

//当位移绝对值较大时才使用定位算法，此时应适当提高测量值的信任度
# define Q_VALUE_RUN 0.0001
# define R_VALUE_RUN 0.01

//对测量值和滤波值偏差的容忍度
//# define TOLERANCE 0.5
//
////偏差连续超过容忍度几次后切换系统
//# define TOLERANCE_COUNT_MAX 2



typedef enum {
	KalmanMode_Init,
	KalmanMode_Run
}KalmanModeTypedef;



/* 2 Dimension */
typedef struct {
    float x[2][1];     /* state: 坐标（x，y） */
    float A[2][2];  /* X(n)=A*X(n-1)+U(n),U(n)~N(0,q), 2x2 */
    float B[2][2];
    float H[2][2];     /* Z(n)=H*X(n)+W(n),W(n)~N(0,r), 1x2   */
    float q[2][2];     /* 过程噪声协方差矩阵：在这里是编码器噪声，process(predict) noise convariance,2x1 [q0,0; 0,q1] */
    float r[2][2];     /* 测量噪声协方差矩阵：在这里是定位算法的噪声，measure noise convariance */
    float p[2][2];  	/*误差协方差矩阵 estimated error convariance,2x2 [p0 p1; p2 p3] */
    float gain[2][2];  	/*卡尔曼增益 */
    float measured_x[2][1];/*测量值 */
    KalmanModeTypedef mode;

    float timestamp;
} kalman2_state;



//初始化，给定初始坐标和误差协方差矩阵
void kalman2_init(kalman2_state *state, float (*init_x)[1], float (*init_p)[2]);

//迭代计算
void kalman2_filter(kalman2_state *state, float (*u)[1], float (*z_measure)[1]);

void mode_switch(kalman2_state *state,float tolerance,uint8_t tolerance_count_max, uint8_t* p_count);
# endif
