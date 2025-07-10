#ifndef MYMAIN_H
#define MYMAIN_H

#include "hrtim.h"
#include "math.h"
#include "LCD.h"
#include "menu.h"
#include "Delay.h"
#include "EEPROM_M24C02.h"


#include "main.h"
#include "control.h"
#include "transform.h"
#include "protection.h"
#include "mode.h"


typedef struct
{
	
	volatile uint8_t Mode;	   	// 运行状态位
	volatile uint8_t State;	   	// 运行状态位
	volatile uint8_t Flag;	   	// 运行标志位
	
}Program_State_Flag;

// 主函数状态机枚举量
typedef enum
{
	Mode_Kong,				//检测模式
	
	Mode_PV_Grid,			//光伏并网
	Mode_PV_Grid_Storage,	//光伏并网储能
	
	Mode_PV_DC_Storage,		//光伏直流储能
	Mode_PV_DC_V,			//光伏直流恒压
	Mode_PV_DC_I,			//光伏直流恒流
	Mode_PV_AC_Storage,		//光伏离网储能
	Mode_PV_AC_V,			//光伏逆变恒压
	Mode_PV_AC_I,			//光伏逆变恒流
	
} Program_Mode;

// 主函数状态机枚举量
typedef enum
{
	State_Standby,	// 待机 标志位
	
	State_Boost,	// 稳压 标志位
	State_Invert,	// 逆变 标志位
	State_Grid_,	// 并网 标志位
	State_Mppt_,	// 追踪 标志位
	
	
	
	State_Err_,		// 故障 标志位
	
} Program_State;

// 主函数状态机枚举量
typedef enum
{
	Flag_Standby,			// 待机 状态
	Flag_Run_,				// 正常 运行
	
	Flag_OCP_Err_DC,		// 过流 错误
	Flag_OVP_Err_DC, 		// 过压 错误
	Flag_OCP_Err_AC,		// 过流 错误
	Flag_OVP_Err_AC, 		// 过压 错误
	
	Flag_DC_Err,		 	// 升压 错误
	Flag_AC_Err,	 		// 逆变 错误
	Flag_GD_Err,	 		// 并网 错误
	
	Flag_LOW_Err,			// 低压 错误
	Flag_ARC_Err,			// 拉弧 错误
	
} Program_Flag;

extern Program_State_Flag Program;


extern uint8_t  M24C02_Vmax[];//最大电压寄存
extern uint8_t  M24C02_vset[];//设定电压寄存
extern uint16_t AD_Buf[];	//DMA转运存储

extern pid_typedef pid_dc_V;
extern pid_typedef pid_value_V;
extern pid_typedef pid_value_I;
extern pid_typedef pid_theta_V;
extern pid_typedef pid_theta_I;
extern pid_typedef pid_theta_G;

extern pid_Mppt Mppt_W;
extern pid_typedef mppt_dc_W;
extern pid_typedef mppt_ac_W;

extern SOGI sogi_Vout;
extern SOGI sogi_Iout;
extern SOGI sogi_Grid;


extern float 	g_D , g_Q;//电网dq解耦转化直流量
extern float	v_D , v_Q;//电压dq解耦转化直流量
extern float	i_D , i_Q;//功率dq解耦转化直流量
extern float sin_0,cos_0;

extern float debug,debug1,debug2,debug3;

extern  int16_t in_V,in_I;		//直流输入
extern uint16_t dc_V,dc_I;		//直流输出
extern int16_t ac_V;			//输出电流
extern int16_t ac_I;			//设定电压
extern int16_t Grid;			//设定电流

extern uint32_t ccr_DC;	//CCR
extern uint32_t ccr_AC;	//CCR

extern float 	value_g,value_v,value_i,value;//幅值跟随
extern float	theta_g,theta_v,theta_i,theta;//相位锁相

extern float show_dcV,show_dcI,show_acV,show_acI;
extern float show_dcW,show_rat,show_acQ,show_THD;

#endif
