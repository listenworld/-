#ifndef INC_TRANSFORM_H_
#define INC_TRANSFORM_H_

#include "main.h"

#define CCMRAM __attribute__((section("ccmram")))

void Slid_Mppt_Filter( uint16_t in_V, uint16_t in_I ,int16_t *dc_V, int16_t *dc_I);
void Slid_Mean_Filter( uint16_t *dc_V, uint16_t *dc_I);

//dq解耦解耦坐标变换
CCMRAM void Park_Transform(float cosout, float sinout,float cosgad, float singad ,float* d,float* q);
CCMRAM void InvPark_Transform(int16_t grid_ad,float value_v, float value_w, float theta, uint16_t *ccr);

typedef struct
{
		int16_t gridarr[200];
		int16_t cnt;
		float value,theta;
		int16_t ccr;
	
}Feedforward_Decoupl;
CCMRAM void FeedPark_V(Feedforward_Decoupl*FD, int16_t *ccr,uint8_t *n,int16_t grid_ad, float value, float theta);
CCMRAM void FeedPark_I(Feedforward_Decoupl*FD, int16_t *ccr,uint8_t  n, float value, float theta);

typedef struct
{
	int16_t maxvalue , minvalue;
	int16_t maxlast  , minlast;
	
	int32_t AVsum;
	int16_t AV;
	int16_t AM;
	
	uint16_t cnt;
	
}Get_value;
CCMRAM void Get_av(Get_value *getAV,int16_t ad_value,int16_t *av_value);
CCMRAM void Get_compute(Get_value *getvalue,int16_t ad_value,int16_t *compute);


typedef struct
{
    float k;            //增益系数
    float alpha;        //alhfa轴分量
    float beta;         //beta轴分量
    float w0;           //角速度
    float Ts;           //时钟周期
    float err;           //时钟周期	
}SOGI;
void SOGI_PLL_Init(SOGI *sogi, float sogi_t);
CCMRAM void SOGI_Update(SOGI *sogi ,float in,float *alpha,float *beta);

#endif

