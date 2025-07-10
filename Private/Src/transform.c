#include "stm32g4xx_hal.h"
#include "math.h"
#include "transform.h"


void Slid_Mppt_Filter( uint16_t in_V, uint16_t in_I ,int16_t *dc_V, int16_t *dc_I)
{
	
	static uint32_t INV_AvgSum = 0 , INI_AvgSum = 0 ;
	
    INV_AvgSum = INV_AvgSum + in_V - (INV_AvgSum >> 10);
    *dc_V = (INV_AvgSum >> 10 )-18;
	
    INI_AvgSum = INI_AvgSum + in_I - (INI_AvgSum >> 10);
    *dc_I = (INI_AvgSum >> 10 )-2061;
	
}
void Slid_Mean_Filter( uint16_t *dc_V, uint16_t *dc_I)
{
	
	static uint32_t DCV_AvgSum = 0 , DCI_AvgSum = 0 ;
	
    DCV_AvgSum = DCV_AvgSum + *dc_V - (DCV_AvgSum >> 2);
    *dc_V = DCV_AvgSum >> 2 ;
	
    DCI_AvgSum = DCI_AvgSum + *dc_I - (DCI_AvgSum >> 10);
    *dc_I = DCI_AvgSum >> 10 ;
	
}

// Park变换：将αβ分量转换到旋转dq坐标系
// theta - 当前电压相位（由PLL获取，单位：弧度）
CCMRAM void Park_Transform(float cosout, float sinout,float cosgad, float singad ,float* d,float* q)
{    
    *d =  cosout * cosgad + sinout * singad;	// d轴分量（无功相关）
    *q = -cosout * singad + sinout * cosgad;	// q轴分量（有功相关）
}

CCMRAM void FeedPark_V(Feedforward_Decoupl*FD, int16_t *ccr,uint8_t *n,int16_t ad_grid, float value, float theta)
{
		if(*n>=200) *n-=200;
		FD->gridarr[*n]=ad_grid;
		FD->cnt=*n+(int16_t)theta;
		FD->cnt = (FD->cnt >= 200 ) ? (FD->cnt - 200) : ((FD->cnt < 000)?(FD->cnt + 200) : FD->cnt);
	
		FD->ccr = (int16_t)(value * FD->gridarr[FD->cnt]/730);
		*ccr = FD->ccr;
}

//CCMRAM void Feedforward(int16_t grid_ad, float value, float theta, uint16_t *ccr)
//{
//		static int16_t gridarr[3000],i=0,cnt=0;
//	
//		if(i==3000) i=0000;
//		gridarr[i]=grid_ad;
//		i++;
//		if(i<1000)cnt=i+1000;
//		if(i<1000)cnt=i+1000;
//	
//		*ccr = (uint16_t)(value*gridarr[cnt+theta]/730);

//}
//CCMRAM void Decoupling(float value, float theta, int16_t n, uint16_t *ccr)
//{
//		*ccr = (uint16_t)(value*sinf(n*3.1415926f/100+theta) + 8500);  // α轴电压指令

//}

//消除采样直流偏置
CCMRAM void Get_av(Get_value *getAV,int16_t ad_value,int16_t *av_value)
{	
	//比较
	getAV->maxvalue = (ad_value > getAV->maxlast) ? ad_value : getAV->maxlast;
	getAV->minvalue = (ad_value < getAV->minlast) ? ad_value : getAV->minlast;
	//记录
	getAV->minlast=getAV->minvalue;
	getAV->maxlast=getAV->maxvalue;
	//重置
	if( getAV->cnt==400)		getAV->cnt=0;
	//计算
	if(getAV->cnt==0)
	{
	getAV->AV=(getAV->maxlast+getAV->minlast)>>1;
	getAV->AM=(getAV->maxlast-getAV->minlast)>>1;
	getAV->AVsum = getAV->AVsum + getAV->AV - (getAV->AVsum >> 2);
	getAV->minlast=getAV->AV;
	getAV->maxlast=getAV->AV;
	}
	//反馈
	*av_value = ad_value - 
//	getAV->AV;
	(getAV->AVsum >> 2);
	getAV->cnt++;
}

CCMRAM void Get_compute(Get_value *getvalue,int16_t ad_value,int16_t *compute)
{
	
	getvalue->AVsum = getvalue->AVsum + ad_value - (getvalue->AVsum >> 10);
	*compute = ad_value - (getvalue->AVsum >> 10);
	
}


void SOGI_PLL_Init(SOGI *sogi, float sogi_t)
{
    sogi->alpha = 0.0f;				//
    sogi->beta = 0.0f;				
    sogi->k = 1.0f;					//
    sogi->w0 = 100.0f *3.14159265f;	//对象角速度
    sogi->Ts = sogi_t;
}

CCMRAM void SOGI_Update(SOGI *sogi ,float in,float *alpha,float *beta)
{
    sogi->err 	= in - sogi->alpha;
    sogi->alpha = sogi->alpha + (sogi->err * sogi->k - sogi->beta) * sogi->w0 * sogi->Ts;
    sogi->beta += sogi->alpha * sogi->w0 * sogi->Ts;
	*alpha=sogi->alpha;
	*beta=sogi->beta;
	
}


