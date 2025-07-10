#ifndef INC_CONTROL_H_
#define INC_CONTROL_H_

#include "main.h"
#define CCMRAM __attribute__((section("ccmram")))

typedef struct
{
	float Set_Value , Actual_Value;
	float err , err_one , err_two;
	float Eer , Eer_one ;	
	float intergral ,pid_out, out;//积分
	float Kp , Ki , Kd;
	float outmax,outmin;
	uint16_t ts;//以k为单位
	
}pid_typedef;
void Reset_PID(pid_typedef *pid);
//位置式
void Init_PID_Pos(pid_typedef *pid,uint16_t TS,float KP,float KI,float KD,float SetValue,float outmax,float outmin);
CCMRAM float PID_Pos(pid_typedef *pid,float ActualValue);
//增量式
void Init_PID_Inc(pid_typedef *pid,uint16_t TS,float KP,float KI,float KD,float SetValue,float outmax,float outmin);
CCMRAM float PID_Inc(pid_typedef *pid,float ActualValue);


typedef struct
{
	float BusMax,BusMin;
	float Key_trend;
	float step,ratio,fixes;//步距，比例，修正
	float Set_P , Act_P;
	float Set_V , Act_V;
	float Set_I , Act_I;
	float d_V	, d_I , d_P;
	float Mppt_out;
	
	float Set_acV , Actual_acV;//交流电压
	float Set_acI , Actual_acI;//交流电流
	
	float err_acV , err_last_acV ;
	float err_acI , err_last_acI ;
	
	uint16_t ts;//以k为单位
	
}pid_Mppt;
void Reset_PID_Mppt_DAC(pid_Mppt *pid);
void Init_PID_Mppt_DAC(pid_Mppt *pid,uint16_t TS,float Step,float Ratio,float Fixes,float busmax,float busmin);
void PID_Mppt_DAC(pid_Mppt *pid,float* CCR_dc,float* CCR_ac,float in_V,float in_I,float dc_V,float ac_I);
//最大功率追踪
void Init_PID_Mppt(pid_typedef *pid,uint16_t Ts,float KP,float KI,float KD,float outmax,float outmin);
CCMRAM float PID_Mppt_DC(pid_typedef *pid,float dc_V,float dc_I);
CCMRAM float PID_Mppt_AC(pid_typedef *pid,float dc_V,float ac_I);

#endif


