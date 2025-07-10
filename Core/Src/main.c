/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
	* @version				: 1.2
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "comp.h"
#include "cordic.h"
#include "dma.h"
#include "hrtim.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "LCD.h"
#include "UART.h"
#include "Mymain.h"
#include "lvgl.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"

#include "gui_guider.h"
#include "events_init.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
int8_t button_pressed_id = -1; // 记录被按下的按键ID

lv_ui guider_ui;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint16_t AD_Buf[5];
uint16_t AD_Mppt[2];

//Program_State_Flag Program={Mode_PV_Grid,State_Standby,Flag_Run_};
uint16_t AD_Buf[5];
uint16_t AD_Mppt[2];

//Program_State_Flag Program={Mode_PV_Grid,State_Standby,Flag_Run_};
Program_State_Flag Program={Mode_PV_Grid,State_Standby,Flag_Standby};

pid_typedef pid_dc_V;
pid_typedef pid_value_V;
pid_typedef pid_value_I;
pid_typedef pid_theta_V;
pid_typedef pid_theta_I;
pid_typedef pid_theta_G;

pid_Mppt Mppt_W;
pid_typedef mppt_dc_W;
pid_typedef mppt_ac_W;

Get_value getVout;
Get_value getIout;
Get_value getGrid;

int16_t in_V,in_I;
uint16_t dc_V,dc_I;
int16_t ac_V,ac_I,Grid;
//[3]-dc_v   [2]-dc_i
//[4]-ac_v   [1]-ac_i   [0]-grid

int16_t vout_ad,	iout_ad,	grid_ad;
int16_t	vout_av,	iout_av,	grid_av;//交流采样

uint16_t 	count_minor=0;
uint8_t 	count_key=0;

float debug =0,debug1=0,debug2=0,debug3=0;

float show_dcV=0.00f,show_dcI=0.00f,show_acV=0.00f,show_acI=0.00f;
float show_rat=0.00f,show_dcW=0.00f,show_acQ=0.82f,show_THD=0.03f;

//ARR=34 000
uint32_t ccr_DC=0;
uint32_t ccr_AC=17000;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_HRTIM1_Init();
  MX_USART1_UART_Init();
  MX_TIM7_Init();
  MX_TIM6_Init();
  MX_SPI3_Init();
  MX_CORDIC_Init();
  MX_ADC2_Init();
  MX_COMP1_Init();
  MX_COMP2_Init();
  /* USER CODE BEGIN 2 */
	/*------------------------------------------EEROM------------------------------------------------*/	
	/*------------------------------------------HRTIM-------------------------------------------------*/
	HAL_HRTIM_WaveformCounterStart(&hhrtim1, HRTIM_TIMERID_TIMER_C|HRTIM_TIMERID_TIMER_E|HRTIM_TIMERID_TIMER_F); //同时启动TIMER_A，TIMER_B的计数器
	HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TC1|HRTIM_OUTPUT_TC2); //使能TIMER_C的波形输出-DC
	HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TE1|HRTIM_OUTPUT_TE2); //使能TIMER_E的波形输出-AC
	HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TF1|HRTIM_OUTPUT_TF2); //使能TIMER_F的波形输出-AC
	
	HAL_TIM_Base_Start_IT(&htim6); // 启动TIM6中断
	HAL_TIM_Base_Start_IT(&htim7); // 启动TIM7中断
	
	/*------------------------------------------ADC---------------------------------------------------*/
	HAL_ADCEx_Calibration_Start(&hadc1,ADC_SINGLE_ENDED);		//ADC1校准
	HAL_ADC_Start_DMA(&hadc1,(uint32_t*)AD_Buf,5);				//开启ADC的DMA模式	
//	HAL_ADCEx_Calibration_Start(&hadc3,ADC_SINGLE_ENDED);		//ADC2校准
//	HAL_ADC_Start_DMA(&hadc3,(uint32_t*)AD_Mppt,2);				//开启ADC的DMA模式	
	

//	HAL_GPIO_WritePin(GPIOE,KEY_Pin,GPIO_PIN_SET);//继电器开
	
	/*------------------------------------------Program--------------------------------------------------*/
	
	Init_PID_Inc(&pid_theta_G,10000 , 5.5f	,1.3f	,9.9f	, 0 , 6.2831853f	, -6.2831853f	);
	
	//空载电压
	Init_PID_Pos(&pid_dc_V	 ,10	, 7777	,11.0f	,12345	, 48.0f*4095/70.35f	, 23800 , 0 	);//1/(1-29143/34000)=7>52/10=5.2
//	Init_PID_Pos(&pid_dc_V	 ,10	, 0		,0		,0		, 48.0f*4095/70.35f	, 23800 , 0 	);//1/(1-29143/34000)=7>52/10=5.2
	Init_PID_Pos(&pid_value_V,10	, 222 	,21		,333	, 24.7f*4095/75.8f	, 15300	, 0		);
	Init_PID_Inc(&pid_theta_V,100	, 4		,5		,13		, 0	, 200	, -200	);
	
	//并网初始电流
	Init_PID_Pos(&pid_value_I,100	, 5		,1.3f	,2		, 0.3f*4095/19.65f	, 10000	, -4000	);
	Init_PID_Inc(&pid_theta_I,10000 , 9		,2.1f	,5		, 0	, 6.2831853f	, -6.2831853f	);
	
	//并网后Mppt
	Init_PID_Mppt_DAC(&Mppt_W,10, 1700 , 1.2f , 1.3f ,25*4095/70.35f,44*4095/70.35f);

	SOGI_PLL_Init(&sogi_Vout,0.0001f);
	SOGI_PLL_Init(&sogi_Iout,0.0001f);
	SOGI_PLL_Init(&sogi_Grid,0.0001f);
	
	/*------------------------------------------LCD---------------------------------------------------*/
	LCD_Init();

//	LCD_Clear(WHITE);
	
	/*------------------------------------------menu-------------------------------------------------*/	
	Menu_Init();
lv_init();
lv_port_disp_init();
lv_port_indev_init();

//	lv_obj_t * btn_led2_obj = lv_btn_create(lv_scr_act());
//	lv_obj_set_size(btn_led2_obj,160,30);
//	lv_obj_align(btn_led2_obj,LV_ALIGN_CENTER,0,0);
//	lv_obj_t * label_obj = lv_label_create(btn_led2_obj);
//	lv_label_set_text(label_obj,"Turn ON LED2");
//	lv_obj_center(label_obj);
//				uint8_t id = lv_tabview_get_tab_act(guider_ui.screen_tabview_1);
//				lv_tabview_set_act(guider_ui.screen_tabview_1,id-1,LV_ANIM_OFF);

setup_ui(&guider_ui);
events_init(&guider_ui);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
//	count_minor++;
	/*------------------------------------------menu---------------------------------------------------*/	
	HAL_Delay(5);
	lv_timer_handler();
	lv_label_set_text_fmt(guider_ui.screen_label_5 , "%.2f",show_dcV);//直流电压
	lv_label_set_text_fmt(guider_ui.screen_label_6 , "%.2f",show_dcI);//直流电流
	lv_label_set_text_fmt(guider_ui.screen_label_7 , "%.2f",show_acV);//交流电压
	lv_label_set_text_fmt(guider_ui.screen_label_8 , "%.2f",show_acI);//交流电流
	lv_label_set_text_fmt(guider_ui.screen_label_16, "%.2f",show_rat);//效率
	lv_label_set_text_fmt(guider_ui.screen_label_15, "%.2f",show_dcW);//功率
	lv_label_set_text_fmt(guider_ui.screen_label_14, "%.2f",show_acQ);//功率因素
	lv_label_set_text_fmt(guider_ui.screen_label_13, "%.2f",show_THD);//THD
		
//		if(Program.State!=State_Err_)Menu_Refresh();

	/*------------------------------------------UART---------------------------------------------------*/

//		Vofa_JustFloat_multiple( 8	  
//	  
////			,value_i
////			,theta_i
//			,(float)ccr_DC
//			,(float)ccr_AC
//			
////			,pid_value_I.out

////			,(float)in_V
////			,(float)in_I
////			,(float)AD_Mppt[0]
////			,(float)AD_Mppt[1]
//			
//			,(float)Grid//电网电压
//			,(float)ac_I//并网电流
//			,(float)ac_V//交流电压
////			,debug
////			,(float)AD_Buf[0]
////			,(float)AD_Buf[1]
////			,(float)AD_Buf[4]
//	  
////			,(float)dc_I//直流电流
//			,(float)dc_V//直流电压
//			,debug3
////			,(float)AD_Buf[2]
////			,(float)AD_Buf[3]
////			,pid_dc_V.err
//			
////			,Mppt_W.Mppt_out
////			,Mppt_W.d_V
////			,Mppt_W.d_I
////			,Mppt_W.Key_trend
////			,mppt_dc_W.out
////			,mppt_ac_W.out
//			
//			,i_D
//			,i_Q
////			,value
////			,v_D
////			,v_Q
////			,pid_value_V.err
////			,pid_theta_V.err
////			,value
////			,debug
////			,g_D
////			,g_Q
//		);

  }	    
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV5;
  RCC_OscInitStruct.PLL.PLLN = 68;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
/*------------------------------------------回调函数---------------------------------------------------*/

/******************************按键检测*************************/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) 
{					/* 按键处理 */
//	Menu_KeyProcess(GPIO_Pin);	


	
}				
					
/******************************中断tim7*************************/	
/* USER CODE BEGIN 1 */

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim == &htim7)//10 000Hz
	{
		//采样信号处理
			//转运采样数据
//			in_V=AD_Mppt[0];//电压橘红
//			in_I=AD_Mppt[1];//电流蓝黄
			Grid=AD_Buf[0];//电网电压
			ac_I=AD_Buf[1];//并网电流
			dc_I=AD_Buf[2];//直流电流
			dc_V=AD_Buf[3];//直流电压
			ac_V=AD_Buf[4];//交流电压
		
			//直流信号滤波
			Slid_Mppt_Filter(AD_Mppt[0],AD_Mppt[1], &in_V, &in_I );
			Slid_Mean_Filter( &dc_V, &dc_I );
		
			/*****************数据显示*****************/
			show_dcV=(float)(dc_V/058.2f);
			show_dcI=(float)(dc_I/930.7f);
			show_dcW=(float)(show_dcV*show_dcI);
		
		
			//消除直流偏置
//			Get_av(&getVout,AD_Buf[4],&ac_V);//交流电压
//			Get_av(&getIout,AD_Buf[1],&ac_I);//并网电流
//			Get_av(&getGrid,AD_Buf[0],&Grid);//电网电压
			Get_compute(&getVout,AD_Buf[4],&ac_V);//交流电压
			Get_compute(&getIout,AD_Buf[1],&ac_I);//并网电流
			Get_compute(&getGrid,AD_Buf[0],&Grid);//电网电压
		
			if(count_key>=200) count_key-=200,count_minor++;
		
		switch (Program.Mode)
			{
			case Mode_PV_Grid :			PV_Grid(&count_key,&ccr_DC,&ccr_AC);
		break;
			case Mode_PV_Grid_Storage :	PV_Grid_Storage(&count_key,&ccr_DC,&ccr_AC);
		break;
			case Mode_PV_DC_Storage :	PV_DC_Storage(&count_key,&ccr_DC,&ccr_AC);
		break;
			case Mode_PV_AC_Storage :	PV_AC_Storage(&count_key,&ccr_DC,&ccr_AC);
		break;
			case Mode_PV_AC_V :	PV_AC_V(&count_key,&ccr_DC,&ccr_AC);
		break;
			case Mode_PV_AC_I :	PV_AC_I(&count_key,&ccr_DC,&ccr_AC);
		break;
			case Mode_PV_DC_V :	PV_DC_V(&count_key,&ccr_DC,&ccr_AC);
		break;
			case Mode_PV_DC_I :	PV_DC_I(&count_key,&ccr_DC,&ccr_AC);
		break;
			}


		
		__HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_C, HRTIM_COMPAREUNIT_1, ccr_DC);
		__HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_E, HRTIM_COMPAREUNIT_1, ccr_AC);
		__HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_F, HRTIM_COMPAREUNIT_1, ccr_AC);

    }
	   if (htim == &htim6)//5 000Hz
	   {
		   
			ERROR_Testing(ccr_DC, ccr_AC, dc_V, dc_I, 0 , 0 , 0 , 0 );
			lv_tick_inc(1);//一毫秒计数
	   }

}
/* USER CODE END 1 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
