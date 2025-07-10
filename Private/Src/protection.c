#include "stm32g4xx.h"                  // Device header
#include "Mymain.h"
#include "protection.h"
#include "Delay.h"

#define DC_Fault 15*4095/70.5		//直流输入最小13V
#define DC_Start 15*4095/70.5		//直流启动最小15V
#define DC_V_max 59*4095/70.5		//直流输出最大56V
#define DC_V_Bus 48*4095/70.5		//直流母线电压42V
#define DC_V_min 35*4095/70.5		//直流输出最小35V

#define DC_I_max 4*15*50/1000*4095/3.3	//直流电流最大3A
#define DC_I_min 0.2f*15*50/1000*4095	//直流电流最小0.2A

#define DC_W_max 100*15*50/1000/47*2.2/3.3*4095	//直流功率最大99W
#define DC_W_min 9.9f*15*50/1000/47*2.2/3.3*4095//直流功率最小10W

#define AC_V_max 100*15*50/1000/47*2.2/3.3*4095	//交流电压
#define AC_V_min 9.9f*15*50/1000/47*2.2/3.3*4095//交流电压

#define AC_W_max 100*15*50/1000/47*2.2/3.3*4095	//交流电流
#define AC_W_min 9.9f*15*50/1000/47*2.2/3.3*4095//交流电流

#define AC_G_bus 26*4095/75.8f//电网阈值


void ERROR_Testing(uint32_t DCccr,uint32_t ACccr,float dc_v,float dc_i,float ac_d,float ac_q,float gd_d,float gd_q)
{
	/*依据标志位做出指令*/
	switch(Program.Flag){
		// 待机 状态
		case Flag_Standby:
			HAL_GPIO_WritePin(GPIOE,KEY_Pin,GPIO_PIN_RESET);//关
	break;
		
		// 正常 运行
		case Flag_Run_:
		
	break;
		
		// 过压 错误
		case Flag_OVP_Err_DC:
//			Program.State=State_Standby;
			HAL_GPIO_WritePin(GPIOE,KEY_Pin,GPIO_PIN_RESET);//关
		
	break;
		
		// 低压 错误
		case Flag_LOW_Err:
//			Program.State=State_Standby;
			HAL_GPIO_WritePin(GPIOE,KEY_Pin,GPIO_PIN_RESET);//关
		
	break;
		
		// 过流 错误
		case Flag_OCP_Err_DC:
//			Program.State=State_Standby;
			HAL_GPIO_WritePin(GPIOE,KEY_Pin,GPIO_PIN_RESET);//关
		
	break;
		
		// 过压 错误
		case Flag_OVP_Err_AC:
//			Program.State=State_Standby;
			HAL_GPIO_WritePin(GPIOE,KEY_Pin,GPIO_PIN_RESET);//关
		
	break;
		
		// 过流 错误
		case Flag_OCP_Err_AC:
//			Program.State=State_Standby;
			HAL_GPIO_WritePin(GPIOE,KEY_Pin,GPIO_PIN_RESET);//关
		
	break;
		
		// 升压 错误
		case Flag_DC_Err:
//			Program.State=State_Standby;
			HAL_GPIO_WritePin(GPIOE,KEY_Pin,GPIO_PIN_RESET);//关
		
	break;
		
		// 逆变 错误
		case Flag_AC_Err:
//			Program.State=State_Standby;
			HAL_GPIO_WritePin(GPIOE,KEY_Pin,GPIO_PIN_RESET);//关
		
	break;
		
		// 并网 错误
		case Flag_GD_Err:
//			Program.State=State_Standby;
			HAL_GPIO_WritePin(GPIOE,KEY_Pin,GPIO_PIN_RESET);//关
			
	break;
		
		// 拉弧 错误
		case Flag_ARC_Err:
//			Program.State=State_Standby;
			HAL_GPIO_WritePin(GPIOE,KEY_Pin,GPIO_PIN_RESET);//关
		
	break;
	}
	
	
	
	static float DCcrK;
	static int16_t Count_Key=0,Count_iq=0;
	
	DCcrK = 1/(1-DCccr/34000);
	
	/*****监测中*****/
	if(Program.State!=State_Err_)
	{
		/*持续监测直流电压*/
		if(dc_v > DC_V_max)			//输出高压故障
			Program.State=State_Err_,Program.Flag=Flag_OVP_Err_DC;
		if(dc_v/DCcrK < DC_V_min)	//输出低压故障
			Program.State=State_Err_,Program.Flag=Flag_LOW_Err;
		
		
		
		/*依据主程序状态更改标志位*/
		switch(Program.State)
		{
/*——————————————————————————————>         状态——待机         <——————————————————————————————*/
			case State_Standby:
				
				HAL_GPIO_WritePin(GPIOE,KEY_Pin,GPIO_PIN_RESET);//关
				
				//待机检测
				if(dc_v/DCcrK > DC_Start&&g_D > 1314)//标志——Run
					Program.State=State_Invert,Program.Flag=Flag_Run_;				//下一步
				break;
				
/*——————————————————————————————>      状态——空载直流逆变      <——————————————————————————————*/
			case State_Invert:
				
//				HAL_GPIO_WritePin(GPIOE,KEY_Pin,GPIO_PIN_SET);//开
				
				//逆变检测
				if(dc_i > DC_I_max)			//标志——直流过压拉弧
					Program.State=State_Err_,Program.Flag=Flag_ARC_Err;
				
				//空载输出(直流|交流)电压有效值误差稳定在0.1V以内&&相位误差稳定5%以内在执行
				if( fabsf(pid_dc_V.err)<13 && fabsf(pid_value_V.err)<13 && fabsf(pid_theta_V.err)<13)Count_Key++;
					if(Count_Key==400) Program.State=State_Grid_,Count_Key=0;		//下一步
				
				break;
				
/*——————————————————————————————>      状态——并网指定电流     <——————————————————————————————*/
			case State_Grid_:	
					
				HAL_GPIO_WritePin(GPIOE,KEY_Pin,GPIO_PIN_SET);//开
			
				//并网检测
				if( dc_i > DC_I_max)			//标志——直流超载
					Program.State = State_Err_,Program.Flag=Flag_OCP_Err_DC;
				if(fabsf(pid_theta_I.err)> 123)Count_iq++;	//标志——相位失真10%
					if(Count_iq==200) Program.State=State_Err_,Program.Flag=Flag_GD_Err;
				if(g_D > AC_G_bus) Program.State=State_Err_,Program.Flag=Flag_OVP_Err_AC;
				
//				//并网电流有效值误差稳定在0.3A&&相位误差稳定5%以内在执行
//				if( fabsf(pid_value_I.err)< 7 && fabsf(pid_theta_V.err)<11 )Count_Key++;
//					if(Count_Key==400) Program.State=State_Mppt_,Count_Key=0;		//下一步
				
				break;
				
/*——————————————————————————————>  状态——最大功率点追踪12~99W  <——————————————————————————————*/
				
			case State_Mppt_:
				
//				if(dc_i > DC_I_max)			//标志——直流超载
//					Program.State=State_Err_,Program.Flag=Flag_OCP_Err_DC;
//				if(dc_i < DC_I_min)			//标志——低压故障
//					Program.State=State_Err_,Program.Flag=Flag_LOW_Err;
			
				break;
			
		}
		
	}else	HAL_GPIO_WritePin(GPIOE,KEY_Pin,GPIO_PIN_RESET);//关
	
	
	
	debug=Count_Key;
}	


