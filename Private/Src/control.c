#include "stm32g4xx_hal.h"

#include "control.h"
#include "math.h"


void Reset_PID(pid_typedef *pid)
{
	
	pid->Actual_Value=0;
	pid->err=0;
	pid->err_one=0;
	pid->err_two=0;
	pid->Eer=0;
	pid->Eer_one=0;
	pid->intergral=0;
	pid->pid_out=0;
	pid->out=0;
}
//位置式
void Init_PID_Pos(pid_typedef *pid,uint16_t Ts,float KP,float KI,float KD,float SetValue,float outmax,float outmin)
{
		pid->Kp=KP/Ts/SetValue*10;			
		pid->Ki=KI/Ts/SetValue;
		pid->Kd=KD/Ts/SetValue*20;
		
		pid->Set_Value=SetValue;
		pid->outmax=outmax;
		pid->outmin=outmin;
		pid->ts=Ts;
}
CCMRAM float PID_Pos(pid_typedef *pid,float ActualValue)
{
		pid->Actual_Value = ActualValue;
	
		pid->err = (pid->Set_Value - pid->Actual_Value);
	
	
		pid->intergral+=pid->Ki * pid->err;
		
		if(pid->intergral>= pid->outmax*1.001f*pid->ts) pid->intergral -= pid->Ki * pid->err;
		if(pid->intergral < pid->outmin*1.001f*pid->ts) pid->intergral -= pid->Ki * pid->err;
		
		pid->out =  pid->Kp * pid->err
				 +  pid->intergral
				 +  pid->Kd * (pid->err - pid->err_one);
		
		pid->err_one = pid->err;

		if(pid->out > pid->outmax)	pid->out = pid->outmax;
		if(pid->out < pid->outmin)	pid->out = pid->outmin;
	
	return pid->out;
}
//增量式
void Init_PID_Inc(pid_typedef *pid,uint16_t Ts,float KP,float KI,float KD,float SetValue,float outmax,float outmin)
{
		pid->Kp = KP/Ts/100;			
		pid->Ki = KI/Ts/100;
		pid->Kd = KD/Ts/100;
		
		pid->outmax=outmax;
		pid->outmin=outmin;
		pid->ts=Ts;
}
CCMRAM float PID_Inc(pid_typedef *pid,float ActualValue)
{
		pid->Actual_Value = ActualValue;
	
		pid->err = pid->Set_Value - pid->Actual_Value;
				
		pid->out += 
					pid->Kp *  pid->err
				 +	pid->Ki * (pid->err-pid->err_one)
				 +  pid->Kd * (pid->err - 2*pid->err_one+pid->err_two);
		
		pid->err_two = pid->err_one;
		pid->err_one = pid->err;

		if(pid->out > pid->outmax)	pid->out -= pid->outmax;
		if(pid->out < pid->outmin)	pid->out -= pid->outmin;
	
	return pid->out;
}

//最大功率点追踪
void Init_PID_Mppt(pid_typedef *pid,uint16_t Ts,float KP,float KI,float KD,float outmax,float outmin)
{
	
		pid->Kp=KP/Ts;			
		pid->Ki=KI/Ts;
		pid->Kd=KD/Ts;
		
		pid->outmax=outmax;
		pid->outmin=outmin;
		pid->ts=Ts;
		
}



CCMRAM float PID_Mppt_DC(pid_typedef *pid,float dc_V,float dc_I)
{
		pid->Set_Value = dc_V*dc_I/1000;
	
		pid->err = (pid->Set_Value - pid->Actual_Value);
		
		pid->intergral+=pid->Ki * pid->err;
	
		//强锁母线电压
		if( dc_V > pid->outmax) pid->intergral -= pid->Ki * pid->err;
		if( dc_V < pid->outmin) pid->intergral -= pid->Ki * pid->err;
		//输出限幅
		if(pid->intergral > 32000)	pid->intergral -= pid->Ki * pid->err;
		if(pid->intergral <-32000)	pid->intergral -= pid->Ki * pid->err;
		pid->out =  pid->Kp * pid->err
				 +  pid->intergral
				 +  pid->Kd * (pid->err - pid->err_one);
		
		pid->err_one = pid->err;
		
		pid->Actual_Value=pid->Set_Value;
		
	return pid->out;		
}

CCMRAM float PID_Mppt_AC(pid_typedef *pid,float dc_V,float ac_I)
{
	static uint8_t count_pid=0;
	//外环低响应
	if(count_pid==0){count_pid=5;
		//电压外环——控制直流母线电压
		pid->Actual_Value=dc_V;
		
		pid->Eer = pid->Set_Value - pid->Actual_Value;
		
		pid->intergral+=pid->Ki * pid->Eer;
		
		if(pid->Actual_Value > pid->outmax) pid->intergral+=pid->Ki * pid->Eer;
		if(pid->Actual_Value < pid->outmin) pid->intergral+=pid->Ki * pid->Eer;
		
		//输出到电流期望 0-4095
		pid->pid_out = pid->Kp * pid->Eer
					 + pid->intergral
					 + pid->Kd * (pid->Eer - pid->Eer_one);
		
		pid->Eer_one = pid->Eer;
	}
	//内环高响应
	//电流内环——控制输出电流
	pid->err = pid->pid_out / 10 - ac_I;//34000/4095~10
	
	//输出到CCR 0-34000
	pid->out+=pid->Kp *  pid->err
			+ pid->Ki * (pid->err-pid->err_one)
			+ pid->Kd * (pid->err - 2*pid->err_one+pid->err_two);
	
	if(pid->out > 13000) pid->out = 13000;
	if(pid->out <-13000) pid->out =-13000;
		
	//误差记录
	pid->err_two = pid->err_one;
	pid->err_one = pid->err;
	
	count_pid--;
	
	return pid->out;
}

void Init_PID_Mppt_DAC(pid_Mppt *pid,uint16_t TS,float Step,float Ratio,float Fixes,float busmax,float busmin)
{
	pid->ts=TS;
	pid->step=Step/TS;
	pid->ratio=Ratio;
	pid->fixes=Fixes;
	pid->BusMax=busmax;
	pid->BusMin=busmax;
}


void Reset_PID_Mppt_DAC(pid_Mppt *pid)
{
        pid->Key_trend = 0.0f;
        pid->Set_P = 0.0f;
        pid->Act_P = 0.0f;
        pid->Set_V = 0.0f;
        pid->Act_V = 0.0f;
        pid->Set_I = 0.0f;
        pid->Act_I = 0.0f;
        pid->d_V = 0.0f;
        pid->d_I = 0.0f;
        pid->d_P = 0.0f;
        pid->Mppt_out = 0.0f;
}

void PID_Mppt_DAC(pid_Mppt *pid,float* CCR_dc,float* CCR_ac,float in_V,float in_I,float dc_V,float ac_I)
{
	static float count_mppt=0;
	
	count_mppt++;//50ms检测一次
	if(count_mppt==1000){count_mppt=0;
	
	pid->Set_V=in_V , pid->d_V=pid->Set_V-pid->Act_V;//电压增量
	pid->Set_I=in_I , pid->d_I=pid->Set_I-pid->Act_I;//电流增量
	pid->Set_P=in_V*in_I , pid->d_P=pid->Set_P-pid->Act_P;//功率增量
	
	if(fabsf(pid->d_V)>1)pid->Key_trend=in_I + in_V * (pid->d_I / pid->d_V);
	
	if(fabsf(pid->d_V) < 2  ){//电压稳定
		//W随I变
		if(pid->d_I > 0)pid->Mppt_out += pid->step;
		else if(pid->d_I < 0 )pid->Mppt_out -= pid->fixes*pid->step;//加快刹车防止电压掉落
			
	}else if(fabsf(pid->d_V) > 11){//电压剧烈抖动
		//功率骤增
		if(pid->Key_trend > 10)pid->Mppt_out+=pid->ratio*pid->step;//大幅追踪
		//功率骤降
		else if(pid->Key_trend <-7)pid->Mppt_out-=pid->ratio*pid->fixes*pid->step;//大幅修正
	}else{//电压微变
		//功率微增
		if(pid->Key_trend > 10)pid->Mppt_out+=pid->step;//小幅追踪
		//功率微降
		else if(pid->Key_trend <-7)pid->Mppt_out-=pid->fixes*pid->step;//小幅修正
	}
	
	pid->Mppt_out++;
	* CCR_dc=pid->Mppt_out;//34000/1000
	pid->Act_V=pid->Set_V;
	pid->Act_I=pid->Set_I;
	}
}

