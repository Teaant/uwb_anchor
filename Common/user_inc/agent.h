# ifndef __AGENT_H__
# define __AGENT_H__


# include "main.h"



//保留过去N组位移和距离信息
//那么线性方程组矩阵有N-1阶
# define N 50



//设置UWB测距时平均值滤波的采样数，应考虑系统的实时性
//目前leader每50ms进行一次poll可以正常运行，每个个体1秒钟左右进行一次坐标更新
//若有A个agent需要与leader测距，则每个个体一次测距花费50ms*M
//这样的话每个个体每隔50ms*M*A时间进行一次坐标更新
# define M 5


# define DEFAULT_LEADER_ID 101

# define GET_ID_HSEM 29
# define DISPLACEMENT_UPDATE_HSEM 28
# define GET_COORDINATE_HSEM 14
# define IMU_UPDATE_HSEM 15


# define UPDATE_COORDINATE_WITH_ID 16

# define MSG_TYPE_QUERY_ID 5
# define MSG_TYPE_CONFIRM_ID 6


typedef struct{
    float x_m;
    float y_m;
}CoordinateTypeDef;

typedef struct{
    float x_m;
    float y_m;
}DisplacementTypeDef;

//leader中保存所有agent的信息，也只有在leader中才构成一循环链表（在CM4中）
typedef struct{
    uint16_t ID;

    struct AgentTypeDef *next;
    //坐标
    CoordinateTypeDef coordinate;
    //过去四次位移
    DisplacementTypeDef displacement[N];

    //过去四次测距信息(与leader的距离)
    float distance[N];

    uint8_t index;

    //leader对链表进行循环测距时用该值记录上次测距完成后的位移
    DisplacementTypeDef sum_displacement;

}AgentTypeDef;


typedef struct {
	struct AgentLiteTypeDef *next;

	uint16_t ID;

	CoordinateTypeDef coordinate;
}AgentLiteTypeDef;


uint8_t in_agents_list(uint16_t id);
uint16_t length_of_agent_list();
uint8_t insert_list(uint16_t id);

void update_coordinate(uint16_t id, CoordinateTypeDef* coordinate);


//矩阵转置
void matrix_transposition(float *from, uint8_t row, uint8_t col, float *to);

//矩阵相乘，row为left的行数，col为left的列数，假设传入参数绝对正确
void matrix_multiply(float *left, float *right, uint8_t left_row, uint8_t left_col, uint8_t right_col, float *to);

//方阵求逆
void matrix_inversion_2_order(float from[2][2], float to[2][2]);

//求数组array前num个数的平均值
float array_average(float* array, uint8_t num);

float distance_compensate(float distance);

//对测距结果进行滑动滤波
void sliding_filter(float *distance);



//CM7
void StartAgentListUpdateTask(void *arg);


# endif
