#include "stm32g4xx_hal.h"
#include "mode.h"

float 	value_g=0,value_v=0,value_i=0,value;//幅值跟随
float 	theta_g=0,theta_v=0,theta_i=0,theta;//相位锁相
float 	vout_alpha,	iout_alpha,	grid_alpha;//真实交流量
float	vout_beta ,	iout_beta ,	grid_beta; //虚拟交流量
float 	g_D= 0 , g_Q=0;//电网dq解耦转化直流量
float 	v_D= 0 , v_Q=0;//电压dq解耦转化直流量
float 	i_D= 0 , i_Q=0;//功率dq解耦转化直流量

float ccr_Dc , ccr_mppt_Dc , ccr_mppt_Ac;
float sin_0,cos_0;

SOGI  sogi_Vout;
SOGI  sogi_Iout;
SOGI  sogi_Grid;

float grid_ccr[200];

int32_t acV_AvgSum = 0 , acI_AvgSum = 0 ;
	


CCMRAM void PV_Grid(uint8_t *key,uint32_t *ccr_dc,uint32_t *ccr_ac)
{
	static int16_t theta_G;

	//主时间波
	grid_ccr[*key]=Grid;
//	grid_ccr[*key]=1200*cos_0;
	
	*key=*key+1;
	cos_0 = cosf((float)(3.14159265f* *key/100) + theta_g);//
	sin_0 = sinf((float)(3.14159265f* *key/100) + theta_g);//
//	cos_0 = cosf((float)(3.14159265f* *key/100));//
//	sin_0 = sinf((float)(3.14159265f* *key/100));//
	
	//波锁相电网
	SOGI_Update(&sogi_Grid ,Grid,&grid_alpha,&grid_beta);//电网
	g_D = (cos_0 * grid_alpha + sin_0 * grid_beta );	// d轴分量（无功相关）
	g_Q = -cos_0 * grid_beta  + sin_0 * grid_alpha ;	// q轴分量（有功相关）
	theta_g = PID_Inc(&pid_theta_G,g_Q);
	
	//主回路程序进程
	switch(Program.State){
		//待机
		case State_Standby:	
			
			Reset_PID(&pid_theta_G);
			Reset_PID(&pid_dc_V);
			Reset_PID(&pid_value_V);
			Reset_PID(&pid_theta_V);
			Reset_PID(&pid_value_I);
			Reset_PID(&pid_theta_I);
			Reset_PID(&mppt_dc_W);
			Reset_PID(&mppt_ac_W);
			Reset_PID_Mppt_DAC(&Mppt_W);

			break;
		
		//逆变
		case State_Invert:
			
				//虚拟相位
				SOGI_Update(&sogi_Vout ,ac_V,&vout_alpha,&vout_beta);//电压
				//电压解耦	
				v_D =  vout_alpha * cos_0 + vout_beta * sin_0;	// d轴分量（无功相关）
				v_Q = -vout_alpha * sin_0 + vout_beta * cos_0;	// q轴分量（有功相关）
		
				//pid 控制
				value_v=PID_Pos(&pid_value_V,v_D);//位置式-d-幅值
				theta_v=PID_Inc(&pid_theta_V,v_Q);//增量式-q-相位
				
				ccr_Dc = PID_Pos(&pid_dc_V,dc_V);
		
				/*****************数据显示*****************/
				acV_AvgSum = acV_AvgSum + v_D - (acV_AvgSum >> 10);
				show_acV = (acV_AvgSum >> 10 )/54.023f;
				
			break;
		//并网
		case State_Grid_:
				//加快直流响应速度
				Init_PID_Pos(&pid_dc_V,1,159,23,77,48.0f*4095/70.33f,23800,0);
				
				SOGI_Update(&sogi_Iout ,ac_I,&iout_alpha,&iout_beta);//电流
				//功率解耦
				i_D =  iout_alpha * cos_0 + iout_beta * sin_0;	// d轴分量（功率）
				i_Q = -iout_alpha * sin_0 + iout_beta * cos_0;	// q轴分量（相位）
				
				//pid 控制
				value_i=PID_Pos(&pid_value_I,i_D);//位置式-d-幅值
				theta_i=PID_Inc(&pid_theta_I,i_Q);//增量式-q-相位
				
				value=value_v+value_i;
				if(value>16000) value=16000,pid_value_I.intergral -= pid_value_I.Ki * pid_value_I.err;
				else if(value<000)value=000,pid_value_I.intergral -= pid_value_I.Ki * pid_value_I.err;
				ccr_Dc = PID_Pos(&pid_dc_V,dc_V);

				/*****************数据显示*****************/
				acV_AvgSum = acV_AvgSum + g_D - (acV_AvgSum >> 10);
				show_acV = (acV_AvgSum >> 10 )/54.023f;
				
				acI_AvgSum = acI_AvgSum + i_D - (acI_AvgSum >> 10);
				show_acI = (acI_AvgSum >> 10 );
				
				show_rat = show_acI*show_acV/show_dcW;
				
			break;
		//最大功率点追踪
		case State_Mppt_:
			
				SOGI_Update(&sogi_Iout ,ac_I,&iout_alpha,&iout_beta);//电流
				//功率解耦
//				i_D =  iout_alpha * cos_0 + iout_beta * sin_0;	//功率
				i_Q = -iout_alpha * sin_0 + iout_beta * cos_0;		//相位
				theta_i=PID_Inc(&pid_theta_I,i_Q);	//增量式-q-交流电流相位
				
//				ccr_mppt_Dc = (int16_t)PID_Mppt_DC(&mppt_dc_W,dc_V,dc_I);//最大功率输出
//				ccr_mppt_Ac = (int16_t)PID_Mppt_AC(&mppt_ac_W,dc_V,i_D );//控制直流母线
				
				PID_Mppt_DAC(&Mppt_W,&ccr_mppt_Dc,&ccr_mppt_Ac,in_V,in_I,dc_V,ac_I);
				
				value=value_v+value_i;
				if(value>16000) value=16000,pid_value_I.intergral -= pid_value_I.Ki * pid_value_I.err;
				else if(value<000)value=000,pid_value_I.intergral -= pid_value_I.Ki * pid_value_I.err;
		
			break;
		//故障
		case State_Err_:
		
			HAL_GPIO_WritePin(GPIOE,KEY_Pin,GPIO_PIN_RESET);//关
			
			*ccr_dc	= 0;
			ccr_Dc	= 0;
			value	= 0;
			theta	= 0;
			value_i = 0;
			value_v = 0;
			theta_G = 0;
		
			break;
		
		}
		
		//防止越界
		theta_G= *key + (int16_t)theta_v;//前馈
		theta_G=(theta_G>=200)?(theta_G-200):((theta_G < 0 )?(theta_G+200):theta_G);
		theta=0.031415926* *key + theta_g + theta_i ;
		theta=(theta>=6.2831853f)?(theta-6.2831853f):((theta<0.0f)?(theta+6.2831853f):theta);	
		
//		theta=0.03141593f* (*key +theta_v) + theta_g + theta_i ;
//		theta=(theta>=6.283185f)?(theta-6.283185f):((theta<0.0f)?(theta+6.283185f):theta);	
		
		*ccr_dc = ( uint16_t )(ccr_Dc+ccr_mppt_Dc);
		*ccr_ac = ( uint16_t )(17000 + value_v * grid_ccr[theta_G] / 1200 + (value_i) * cosf( theta )) ;
		
//		*ccr_ac = ( uint16_t )(17000 + (value_v + value_i) * cosf( theta )) ;
//		debug2=value_v * grid_ccr[theta_G] / 1200;
		debug3=(value_i) * cosf( theta );

}

CCMRAM void PV_Grid_Storage(uint8_t *key,uint32_t *ad_value,uint32_t *av_value)
{
	
	switch(Program.State){
		//待机
		case State_Standby:	
					
			break;
		//逆变
		case State_Invert:
								
			break;
		//并网
		case State_Grid_:
			
			break;
		//最大功率点追踪
		case State_Mppt_:
		
			break;
		//故障
		case State_Err_:
				
			break;
		
		}
}
CCMRAM void PV_AC_Storage(uint8_t *key,uint32_t *ad_value,uint32_t *av_value)
{
	switch(Program.State){
		//待机
		case State_Standby:	
					
			break;
		//逆变+稳压
		case State_Invert:
								
			break;
		//最大功率点追踪
		case State_Mppt_:
		
			break;
		//故障
		case State_Err_:
				
			break;
		
		}	
}
CCMRAM void PV_DC_Storage(uint8_t *key,uint32_t *ad_value,uint32_t *av_value)
{
	switch(Program.State){
		//待机
		case State_Standby:	
					
			break;
		//逆变+Boost
		case State_Boost:
								
			break;
		//最大功率点追踪
		case State_Mppt_:
		
			break;
		//故障
		case State_Err_:
				
			break;
		
		}		
}
CCMRAM void PV_AC_V(uint8_t *key,uint32_t *ad_value,uint32_t *av_value)
{
	switch(Program.State){
		//待机
		case State_Standby:	
					
			break;
		//交流稳压
		case State_Boost:
								
			break;
		//最大功率点追踪
		case State_Mppt_:
		
			break;
		//故障
		case State_Err_:
				
			break;
		
		}		
}
CCMRAM void PV_AC_I(uint8_t *key,uint32_t *ad_value,uint32_t *av_value)
{
	switch(Program.State){
		//待机
		case State_Standby:	
					
			break;
		//交流稳流
		case State_Boost:
								
			break;
		//最大功率点追踪
		case State_Mppt_:
		
			break;
		//故障
		case State_Err_:
				
			break;
		
		}
}
CCMRAM void PV_DC_V(uint8_t *key,uint32_t *ad_value,uint32_t *av_value)
{
	switch(Program.State){
		//待机
		case State_Standby:	
					
			break;
		//直流稳压
		case State_Boost:
								
			break;
		//最大功率点追踪
		case State_Mppt_:
		
			break;
		//故障
		case State_Err_:
				
			break;
		
		}	
}
CCMRAM void PV_DC_I(uint8_t *key,uint32_t *ad_value,uint32_t *av_value)
{
	switch(Program.State){
		//待机
		case State_Standby:	
					
			break;
		//直流稳流
		case State_Boost:
								
			break;
		//最大功率点追踪
		case State_Mppt_:
		
			break;
		//故障
		case State_Err_:
				
			break;
		
		}	
}


