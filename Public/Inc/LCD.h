#ifndef _SPI_LCD_H
#define	_SPI_LCD_H

#include "stm32g4xx.h"                  // Device header

extern SPI_HandleTypeDef hspi3;


#define LCD_CS(x)  		HAL_GPIO_WritePin(GPIOD,GPIO_PIN_1,(GPIO_PinState)x)//CS
#define LCD_RES(x)  	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,(GPIO_PinState)x)//RES
#define LCD_DC(x)   	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_5,(GPIO_PinState)x)//DC 		
#define LCD_BLK(x) 		HAL_GPIO_WritePin(GPIOD,GPIO_PIN_3,(GPIO_PinState)x)//BLK

#define LCD_H 320
#define LCD_W 240

#define	HAL_MAX_SPI_DELAY		10

//LCD重要参数集
typedef struct  
{										    
	uint16_t 	Width;				//LCD 宽度
	uint16_t 	Height;				//LCD 高度
	uint16_t 	ID;				  	//LCD ID
	uint8_t  	Dir;			  	//横屏还是竖屏控制：0，竖屏；1，横屏。	
	uint16_t	Wramcmd;			//开始写gram指令
	uint16_t  Set_x_cmd;		//设置x坐标指令
	uint16_t  Set_y_cmd;		//设置y坐标指令	
  uint8_t   X_offset;    
  uint8_t	 	Y_offset;
}LCD_PARAM_S; 	

//LCD参数
extern LCD_PARAM_S LCD_param;	//管理LCD重要参数
//TFTLCD部分外要调用的函数		   
extern uint16_t  POINT_COLOR;//默认红色    
extern uint16_t  BACK_COLOR; //背景颜色.默认为白色


/////////////////////////////////////*用户配置区*///////////////////////////////////	 
#define USE_HORIZONTAL  	 3 //定义液晶屏顺时针旋转方向 	0-0度旋转，1-90度旋转，2-180度旋转，3-270度旋转


//画笔颜色

#define WHITE               0x0000      // 白色
#define BLACK               0xFFFF      // 黑色
#define BLUE                0xFFE0      // 蓝色
#define BRED                0x07E0      // 棕红色
#define GBLUE               0xF800      // 绿蓝色
#define RED                 0x07FF      // 红色
#define MAGENTA             0x07E0      // 品红色
#define GREEN               0xF81F      // 绿色
#define CYAN                0x8000      // 青色
#define YELLOW              0x001F      // 黄色
#define BROWN               0x43BF      // 棕色
#define BRRED               0x03F8      // 棕红色
#define GRAY                0x7BCF      // 灰色
#define DARKBLUE            0xFEE0      // 深蓝色
#define LIGHTBLUE           0x8283      // 浅蓝色
#define GRAYBLUE            0xABC7      // 灰蓝色
#define LIGHTGREEN          0x7BE0      // 浅绿色
#define LGRAY               0x39E7      // 浅灰色
#define LGRAYBLUE           0x59AE      // 浅灰蓝色
#define LBBLUE              0xD4ED      // 浅棕蓝色
			  
void LCD_WriteREG(uint8_t cmd);
void LCD_WriteData(uint8_t data);
void LCD_WriteData_Buffer(const uint8_t *data, uint32_t size); 
void LCD_WriteReg(uint8_t LCD_Reg, uint8_t LCD_RegValue);
void LCD_WriteRAM_Prepare(void);
void LCD_WriteData_16Bit(uint16_t Data);
void LCD_DrawPoint(uint16_t x,uint16_t y,uint16_t Point_Color);
void LCD_DrawRegion(uint16_t X,uint16_t Y,uint16_t x,uint16_t y,uint16_t Point_Color);
void LCD_Clear(uint16_t Color);
void LCD_RESET(void);	 
void LCD_Init(void);
void LCD_SetWindows(uint16_t x_Star, uint16_t y_Star,uint16_t x_End,uint16_t y_End);
void LCD_color_fill(uint16_t x_Star, uint16_t y_Star, uint16_t x_End, uint16_t y_End, uint16_t *color);
void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos);
void LCD_direction(uint8_t direction);

void LCD_ShowChar(uint16_t x,uint16_t y,uint16_t fc, uint16_t bc, uint8_t num,uint8_t size,uint8_t mode);
void LCD_ShowString(uint16_t x, uint16_t y, uint8_t size, const char *str, uint8_t mode);
void LCD_HexNum(uint8_t x, uint8_t y,uint8_t size, uint32_t Number, uint8_t Length, uint8_t mode);
void LCD_BinNum(uint8_t x, uint8_t y,uint8_t size, uint32_t Number, uint8_t Length, uint8_t mode);
void LCD_Unsigned_Num(uint8_t x, uint8_t y,uint8_t size, uint32_t Number, uint8_t mode);
void LCD_Signed_Num(uint8_t x, uint8_t y,uint8_t size, uint32_t Number, uint8_t mode);
void LCD_FNum(uint8_t x, uint8_t y,uint8_t size, float Fnum, uint8_t mode);

void LCD_ShowChinese(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, const char *chinese, uint8_t size, uint8_t mode);

void LCD_ShowPicture(uint16_t x,uint16_t y,uint16_t Height,uint16_t Width,const uint8_t pic[]);

#endif
