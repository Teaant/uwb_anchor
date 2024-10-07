#include "utilities.h"
#include "stm32h7xx_hal.h"
#include <string.h>
#include <math.h>

void selection_sort(float *a, int *b, int len)
{
	int i,j;
	for (i=0;i<len;i++)
	{
		float max = *(a+i);
		int pos = 0;
		uint8_t flag = 0;
		for(j=i+1;j<len;j++)
		{
			if(*(a+j)>max || isnan(max))
			{
				max=*(a+j);
				pos=j;
				flag=1;
			}
		}
		if(flag)
		{
			switch_value(a+i,a+pos,sizeof(int));
			switch_value(b+i,b+pos,sizeof(int));
		}
	}
}

void switch_value (void *a, void *b, int len)
{
	uint8_t temp[len];
	memcpy(temp,a,len);
	memcpy(a,b,len);
	memcpy(b,temp,len);
}

int sign(float a)
{
	if (a>0)
		return 1;
	if (a==0)
		return 0;
	if (a<0)
		return -1;
	return 0;
}

float angle_regulation (float a)
{
	while (a>180)	a-=360;
	while (a<-180)	a+=360;
	return a;
}
// 求a在逆时针方向上比b大的角度
float get_angle_diff(float a, float b)
{
	if (abs(a - b) > 180.0)
		if (a > b)
			return a - (b + 360);
		else
			return (a + 360) - b;
	else
		return a-b;
}

float get_angle_sum(float a, float b)
{
	if (abs(a - b) > 180.0)
		return a + b + 360;
	else
		return a + b;
}

float min(float a, float b)
{
	if(a > b)
		return b;
	else
		return a;
}

float max(float a, float b)
{
	if(a > b)
		return a;
	else
		return b;
}
