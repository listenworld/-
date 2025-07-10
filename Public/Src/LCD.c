#include "stdio.h"
#include "stdlib.h"
#include <string.h>
#include "LCD.h"
#include "LCD_Font.h"  	 
#include "Image.h"

//管理LCD重要参数结构体
//默认为竖屏
LCD_PARAM_S LCD_param;

//字符像素颜色缓存区:64为英文字符最大尺寸，8为一个字节的位数,2是因为一个颜色有两个字节
//最大显示32尺寸的汉字
uint8_t LCD_Buffer[2048]={0};


//画笔颜色,背景颜色
uint16_t POINT_COLOR = BLACK; 
uint16_t BACK_COLOR	 = LGRAY; 

/**
  * @brief  向LCD寄存器写入命令
  * @param  cmd: 要写入的寄存器命令
  * @retval 无
  */
void LCD_WriteREG(uint8_t cmd) 
{
    LCD_DC(0);  // DC=0:命令模式
    LCD_CS(0);  // 片选使能
    HAL_SPI_Transmit(&hspi3, &cmd, 1, HAL_MAX_SPI_DELAY);//发送数据
    LCD_CS(1);  // 片选禁止
}

/**
  * @brief  向LCD写入单字节数据
  * @param  data: 要写入的数据
  * @retval 无
  */
void LCD_WriteData(uint8_t data) 
{
    LCD_DC(1);  // DC=1:数据模式
    LCD_CS(0);  // 片选使能
    HAL_SPI_Transmit(&hspi3, &data, 1, HAL_MAX_SPI_DELAY);//发送数据
    LCD_CS(1);  // 片选禁止
}

/**
  * @brief  通过SPI写入数据缓冲区
  * @param  data: 要写入的数据缓冲区指针
  * @param  size: 数据大小（字节数）
  * @retval 无
  */
void LCD_WriteData_Buffer(const uint8_t *data, uint32_t size) 
{
    LCD_DC(1);
    LCD_CS(0);
//    HAL_SPI_Transmit(&hspi3, data, size, HAL_MAX_SPI_DELAY); 
		while(HAL_SPI_GetState(&hspi3)==HAL_SPI_STATE_BUSY_TX){}
		HAL_SPI_Transmit_DMA(&hspi3,data,size);
	
//    LCD_CS(1);
}

/**
  * @brief  向指定寄存器写入数据
  * @param  LCD_Reg: 寄存器地址
  * @param  LCD_RegValue: 要写入的寄存器值
  * @retval 无
  */
void LCD_WriteReg(uint8_t LCD_Reg, uint8_t LCD_RegValue)
{	
	LCD_WriteREG(LCD_Reg);  
	LCD_WriteData(LCD_RegValue);	    		 
}	   

/**
  * @brief  准备写入GRAM（显存）
  * @retval 无
  */
void LCD_WriteRAM_Prepare(void)
{
	LCD_WriteREG(LCD_param.Wramcmd);
}	 

/**
  * @brief  写入16位颜色数据
  * @param  Data: 16位颜色值
  * @retval 无
  */
void LCD_WriteData_16Bit(uint16_t Data)
{
	uint8_t arr[2];
	arr[0]=Data>>8;
	arr[1]=Data;
	
   LCD_CS(0);
   LCD_DC(1); 
   HAL_SPI_Transmit(&hspi3, arr, 2, HAL_MAX_SPI_DELAY);//发送数据
   LCD_CS(1);
}

/**
  * @brief  在指定位置绘制一个像素点
  * @param  x: X坐标
  * @param  y: Y坐标
  * @param  Point_Color: 像素点颜色
  * @retval 无
  */
void LCD_DrawPoint(uint16_t x,uint16_t y,uint16_t Point_Color)
{
	LCD_SetCursor(x,y);//设置光标位置 
	LCD_WriteData_16Bit(Point_Color); 
}

/**
  * @brief  在指定区域绘制像素点
  * @param  x: X坐标
  * @param  y: Y坐标
  * @param  Point_Color: 像素点颜色
  * @retval 无
  */
void LCD_DrawRegion(uint16_t X,uint16_t Y,uint16_t x,uint16_t y,uint16_t Point_Color)
{
	LCD_SetWindows(X,Y,x,y);//设置光标位置 
	LCD_WriteData_16Bit(Point_Color); 
}

/**
  * @brief  清屏函数
  * @param  Color: 清屏颜色
  * @retval 无
  */
void LCD_Clear(uint16_t Color)
{
  uint32_t i,m;  
	LCD_SetWindows(0,0,LCD_param.Width-1,LCD_param.Height-1);   
	LCD_CS(0);
	LCD_DC(1);
	for(i=0;i<LCD_param.Height;i++)
	{
    for(m=0;m<LCD_param.Width;m++)
    {	
			LCD_WriteData_16Bit(Color);
		}
	}
	LCD_CS(1);
} 

/**
  * @brief  LCD硬件复位
  * @retval 无
  */
void LCD_RESET(void)
{
	LCD_RES(0);
	HAL_Delay(20);	
	LCD_RES(1);
	HAL_Delay(20);
}

/**
  * @brief  LCD初始化
  * @retval 无
  */	 
void LCD_Init(void)
{  
 	LCD_RESET(); //LCD 复位
//************* ST7789初始化**********//	
	LCD_WriteREG(0x36); 
	LCD_WriteData(0x00);

	LCD_WriteREG(0x3A); 
	LCD_WriteData(0x05);

	LCD_WriteREG(0xB2);
	LCD_WriteData(0x0C);
	LCD_WriteData(0x0C);
	LCD_WriteData(0x00);
	LCD_WriteData(0x33);
	LCD_WriteData(0x33);

	LCD_WriteREG(0xB7); 
	LCD_WriteData(0x35);  

	LCD_WriteREG(0xBB);
	LCD_WriteData(0x19);

	LCD_WriteREG(0xC0);
	LCD_WriteData(0x2C);

	LCD_WriteREG(0xC2);
	LCD_WriteData(0x01);

	LCD_WriteREG(0xC3);
	LCD_WriteData(0x12);   

	LCD_WriteREG(0xC4);
	LCD_WriteData(0x20);  

	LCD_WriteREG(0xC6); 
	LCD_WriteData(0x0F);    

	LCD_WriteREG(0xD0); 
	LCD_WriteData(0xA4);
	LCD_WriteData(0xA1);

	LCD_WriteREG(0xE0);
	LCD_WriteData(0xD0);
	LCD_WriteData(0x04);
	LCD_WriteData(0x0D);
	LCD_WriteData(0x11);
	LCD_WriteData(0x13);
	LCD_WriteData(0x2B);
	LCD_WriteData(0x3F);
	LCD_WriteData(0x54);
	LCD_WriteData(0x4C);
	LCD_WriteData(0x18);
	LCD_WriteData(0x0D);
	LCD_WriteData(0x0B);
	LCD_WriteData(0x1F);
	LCD_WriteData(0x23);

	LCD_WriteREG(0xE1);
	LCD_WriteData(0xD0);
	LCD_WriteData(0x04);
	LCD_WriteData(0x0C);
	LCD_WriteData(0x11);
	LCD_WriteData(0x13);
	LCD_WriteData(0x2C);
	LCD_WriteData(0x3F);
	LCD_WriteData(0x44);
	LCD_WriteData(0x51);
	LCD_WriteData(0x2F);
	LCD_WriteData(0x1F);
	LCD_WriteData(0x1F);
	LCD_WriteData(0x20);
	LCD_WriteData(0x23);

//	LCD_WriteREG(0x21); 

	LCD_WriteREG(0x11); 
	//Delay (120); 

	LCD_WriteREG(0x29); 	
  LCD_direction(USE_HORIZONTAL);//设置LCD显示方向
	LCD_BLK(1);//点亮背光	 
	LCD_Clear(WHITE);//清全屏白色
}
 
/**
  * @brief  设置显示窗口区域
  * @param  x_Star: 起始X坐标
  * @param  y_Star: 起始Y坐标
  * @param  x_End: 结束X坐标
  * @param  y_End: 结束Y坐标
  * @retval 无
  */
void LCD_SetWindows(uint16_t x_Star, uint16_t y_Star,uint16_t x_End,uint16_t y_End)
{	
	
	LCD_WriteREG(LCD_param.Set_x_cmd);	
	LCD_WriteData((x_Star+LCD_param.X_offset)>>8);
	LCD_WriteData(x_Star+LCD_param.X_offset);		
	LCD_WriteData((x_End+LCD_param.X_offset)>>8);
	LCD_WriteData(x_End+LCD_param.X_offset);

	LCD_WriteREG(LCD_param.Set_y_cmd);	
	LCD_WriteData((y_Star+LCD_param.Y_offset)>>8);
	LCD_WriteData(y_Star+LCD_param.Y_offset);		
	LCD_WriteData((y_End+LCD_param.Y_offset)>>8);
	LCD_WriteData(y_End+LCD_param.Y_offset);

	LCD_WriteRAM_Prepare();	//开始写入GRAM			
}   

void LCD_color_fill(uint16_t x_Star, uint16_t y_Star, uint16_t x_End, uint16_t y_End, uint16_t *color)
{
	LCD_SetWindows( x_Star,  y_Star, x_End, y_End);
	uint16_t Width=x_End-x_Star+1;
	uint16_t Hight=y_End-y_Star+1;
	
//	LCD_WriteData_Buffer((uint8_t *)color,Width*Hight*2);
	for(uint16_t i=0;i<Width;i++)
	{
		for(uint16_t j=0;j<Hight;j++)
		{
			LCD_WriteData_16Bit(color[i*Hight+j]);
		}		
	}
}	
/**
  * @brief  设置光标位置（单点）
  * @param  Xpos: X坐标
  * @param  Ypos: Y坐标
  * @retval 无
  */
void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos)
{	  	    			
	LCD_SetWindows(Xpos,Ypos,Xpos,Ypos);	
} 

/**
  * @brief  设置LCD显示方向
  * @param  direction: 显示方向参数
  * 0x36H寄存器参数说明:
  * D7:MY(行地址顺序)  控制行地址的扫描方向
  * D6:MX(列地址顺序)  控制列地址的扫描方向
  * D5:MV(行列交换)    决定是否交换行和列的地址顺序
  * D4:ML(垂直刷新顺序) '0' 表示从上到下刷新，'1' 表示从下到上刷新
  * D3:RGB(颜色选择器) '0' 表示使用 RGB 颜色滤波面板，'1' 表示使用 BGR 颜色滤波面板
  * D2:MH(水平刷新顺序) '0' 表示从左到右刷新，'1' 表示从右到左刷新
  * @retval 无
  */
void LCD_direction(uint8_t direction)
{ 
			LCD_param.Set_x_cmd=0x2A;
			LCD_param.Set_y_cmd=0x2B;
			LCD_param.Wramcmd=0x2C;
	switch(direction){		  
		case 0:						 	 		
			LCD_param.Width=LCD_W;
			LCD_param.Height=LCD_H;	
			LCD_param.X_offset=0;
			LCD_param.Y_offset=0;
			LCD_WriteReg(0x36,0);//RGB==0,MY==0,MX==0,MV==0
		break;
		case 1:
			LCD_param.Width=LCD_H;
			LCD_param.Height=LCD_W;
			LCD_param.X_offset=0;
			LCD_param.Y_offset=0;
			LCD_WriteReg(0x36,(1<<5)|(1<<6));//RGB==0,MY==1,MX==0,MV==1
		break;
		case 2:						 	 		
			LCD_param.Width=LCD_W;
			LCD_param.Height=LCD_H;
			LCD_param.X_offset=0;
			LCD_param.Y_offset=0;			
			LCD_WriteReg(0x36,(1<<6)|(1<<7));//RGB==0,MY==0,MX==0,MV==0
		break;
		case 3:
			LCD_param.Width=LCD_H;
			LCD_param.Height=LCD_W;
			LCD_param.X_offset=0;
			LCD_param.Y_offset=0;
			LCD_WriteReg(0x36,(1<<7)|(1<<5));//RGB==0,MY==1,MX==0,MV==1
		break;	
		default:break;
	}		
}	 
/*------------------------文字和图片部分------------------------*/

/**
  * @brief  显示单个英文字符
  * @param  x: 起始X坐标
  * @param  y: 起始Y坐标
  * @param  fc: 前景色
  * @param  bc: 背景色
  * @param  num: 要显示的字符（ASCII值）
  * @param  size: 字体大小（12/16/24/32）
  * @param  mode: 显示模式（0:覆盖模式，1:叠加模式）
  * @retval 无
  */
void LCD_ShowChar(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, uint8_t num, uint8_t size, uint8_t mode)
{  
    num = num - ' ';  // 得到偏移后的值
    uint8_t size_width = size / 2;  // 字符宽度
    uint16_t buffer_size = size * size_width * 2;  // 缓冲区大小
    
    // 设置单个文字显示窗口
    LCD_SetWindows(x, y, x + size_width - 1, y + size - 1);
    
    if(!mode)  // 非叠加方式
    {
        uint32_t buffer_index = 0;
        
        for(uint8_t i = 0; i < size; i++)  // 行循环
        {
            uint16_t char_data = 0;
            
            // 获取字模数据
            if(size == 12)char_data = ascii_1206[num][i];
            else if(size == 16)char_data = ascii_1608[num][i];
            else if(size == 24)char_data = (ascii_2412[num][i * 2 + 1] << 8) | ascii_2412[num][i * 2];// 24像素宽字体使用两个字节
            else if(size == 32)char_data = (ascii_3216[num][i * 2 + 1] << 8) | ascii_3216[num][i * 2];// 32像素宽字体使用两个字节
            
            // 处理一行中的所有像素
            for(uint8_t j = 0; j < size_width; j++) 
            {
                if(char_data & (1 << j)) {
                    LCD_Buffer[buffer_index++] = fc >> 8;
                    LCD_Buffer[buffer_index++] = fc;
                } else {
                    LCD_Buffer[buffer_index++] = bc >> 8;
                    LCD_Buffer[buffer_index++] = bc;
                }
            }
        }
        // 发送数据
				LCD_WriteData_Buffer(LCD_Buffer,buffer_size);
    }
    else  // 叠加方式
    {
        for(uint8_t i = 0; i < size; i++)  // 行循环
        {
            uint16_t char_data = 0;
            
            // 获取字模数据
            if(size == 12) {
                char_data = ascii_1206[num][i];
            } else if(size == 16) {
                char_data = ascii_1608[num][i];
            } else if(size == 24) {
                char_data = (ascii_2412[num][i * 2 + 1] << 8) | ascii_2412[num][i * 2];
            } else if(size == 32) {
                char_data = (ascii_3216[num][i * 2 + 1] << 8) | ascii_3216[num][i * 2];
            }
            
            // 处理一行中的所有像素
            for(uint8_t j = 0; j < size_width; j++) 
            {
                if(char_data & (1 << j)) {
                    LCD_DrawPoint(x + j, y + i, fc);  // 只绘制前景色像素
                }
            }
        }
    }
}

/**
  * @brief  显示字符串
  * @param  x: 起始X坐标
  * @param  y: 起始Y坐标
  * @param  size: 字体大小
  * @param  str: 要显示的字符串
  * @param  mode: 显示模式（0:覆盖模式，1:叠加模式）
  * @retval 无
  */
void LCD_ShowString(uint16_t x, uint16_t y, uint8_t size, const char *str, uint8_t mode)
{         
    if(!str) return;  // 空指针检查
    
    uint8_t char_width = size / 2;  // 字符宽度
    
    while(*str != '\0')  // 正确检测字符串结束
    {   
        // 边界检查
        if(x >= LCD_param.Width || y >= LCD_param.Height) 
            return;
        
        // 自动换行处理
        if(x + char_width > LCD_param.Width) {
            x = 0;
            y += size;
            if(y >= LCD_param.Height) return;
        }
        
        // 显示字符
        LCD_ShowChar(x, y, POINT_COLOR, BACK_COLOR, *str, size, mode);
        
        // 移动到下一个位置
        x += char_width;
        str++;
    }  
}

/**
  * @brief  计算幂次方
  * @param  X: 底数
  * @param  Y: 指数
  * @retval 计算结果
  */
uint32_t LCD_Pow(uint32_t X, uint32_t Y)
{
	uint32_t Result = 1;
	while (Y--)
	{
		Result *= X;
	}
	return Result;
}

/**
  * @brief  显示十六进制数字
  * @param  x: 起始X坐标
  * @param  y: 起始Y坐标
  * @param  size: 字体大小
  * @param  Number: 要显示的数字
  * @param  Length: 显示长度
  * @param  mode: 显示模式
  * @retval 无
  */
void LCD_HexNum(uint8_t x, uint8_t y,uint8_t size, uint32_t Number, uint8_t Length, uint8_t mode)
{
	uint8_t i, SingleNumber;
	for (i = 0; i < Length; i++)
	{
		SingleNumber = Number / LCD_Pow(16, Length - i - 1) % 16;
		if (SingleNumber < 10)
		{
      LCD_ShowChar(x + i * size/2,y,POINT_COLOR,BACK_COLOR,SingleNumber + '0',size,mode);
		}
		else
		{
      LCD_ShowChar(x + i * size/2,y,POINT_COLOR,BACK_COLOR,SingleNumber - 10 + 'A',size,mode);
		}
	}
}

/**
  * @brief  显示二进制数字
  * @param  x: 起始X坐标
  * @param  y: 起始Y坐标
  * @param  size: 字体大小
  * @param  Number: 要显示的数字
  * @param  Length: 显示长度
  * @param  mode: 显示模式
  * @retval 无
  */
void LCD_BinNum(uint8_t x, uint8_t y,uint8_t size, uint32_t Number, uint8_t Length, uint8_t mode)
{
	uint8_t i;
	for (i = 0; i < Length; i++)							
	{
    LCD_ShowChar(x + i * size/2,y,POINT_COLOR,BACK_COLOR,Number / LCD_Pow(2, Length - i - 1) % 2 + '0',size,mode);
	}
}

/**
  * @brief  显示无符号整数
  * @param  x: 起始X坐标
  * @param  y: 起始Y坐标
  * @param  size: 字体大小
  * @param  Number: 要显示的数字
  * @param  mode: 显示模式
  * @retval 无
  */
void LCD_Unsigned_Num(uint8_t x, uint8_t y,uint8_t size, uint32_t Number, uint8_t mode)
{
	char Data[10]= {0};
	sprintf(Data , "%u" , Number);
	LCD_ShowString( x, y, size, Data, mode);
}

/**
  * @brief  显示有符号整数
  * @param  x: 起始X坐标
  * @param  y: 起始Y坐标
  * @param  size: 字体大小
  * @param  Number: 要显示的数字
  * @param  mode: 显示模式
  * @retval 无
  */
void LCD_Signed_Num(uint8_t x, uint8_t y,uint8_t size, uint32_t Number, uint8_t mode)
{
	char Data[10]= {0};
	sprintf(Data , "%d" , Number);
	LCD_ShowString( x, y, size, Data, mode);
}

/**
  * @brief  显示浮点数
  * @param  x: 起始X坐标
  * @param  y: 起始Y坐标
  * @param  size: 字体大小
  * @param  Fnum: 要显示的浮点数
  * @param  mode: 显示模式
  * @retval 无
  */
void LCD_FNum(uint8_t x, uint8_t y,uint8_t size, float Fnum, uint8_t mode)
{
	char Data[10]= {0};
	sprintf(Data , "%.3f" , Fnum);
	LCD_ShowString( x, y, size, Data, mode);
}

/**
  * @brief  中文字符显示缓冲区处理（内部函数）
  * @param  x: 起始X坐标
  * @param  y: 起始Y坐标
  * @param  fc: 前景色
  * @param  bc: 背景色
  * @param  data: 字模数据指针
  * @param  size: 字体大小
  * @param  mode: 显示模式
  * @retval 无
  */
static void LCD_chinese_Buffer(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, uint8_t *data, uint8_t size, uint8_t mode)
{  
    uint16_t buffer_size = size * size * 2;  // 缓冲区大小
    
    // 设置单个文字显示窗口
    LCD_SetWindows(x, y, x + size - 1, y + size - 1);
    
    if(!mode)  // 非叠加方式
    {
        uint32_t buffer_index = 0;
        
        for(uint8_t i = 0; i < size; i++)  // 行循环
        {
            uint32_t char_data = 0;
						uint16_t byte_index = i * (size / 8);  // 动态计算起始索引
            
            // 获取字模数据
						if (size == 12 || size == 16)char_data = (data[i*2 + 1] << 8) | data[i*2];// 12/16像素：每行2字节
						else if (size == 24)char_data = (data[byte_index + 2] << 16) | (data[byte_index + 1] << 8) | data[byte_index];// 24像素：每行3字节（小端模式：低地址->低字节）
						else if (size == 32)char_data = (data[byte_index + 3] << 24) | (data[byte_index + 2] << 16) | (data[byte_index + 1] << 8) | data[byte_index];// 32像素：每行4字节（小端模式）

						// 处理一行中的所有像素
            for(uint8_t j = 0; j < size; j++) 
            {
                if(char_data & (1 << j)) {
                    LCD_Buffer[buffer_index++] = fc >> 8;
                    LCD_Buffer[buffer_index++] = fc;
                } else {
                    LCD_Buffer[buffer_index++] = bc >> 8;
                    LCD_Buffer[buffer_index++] = bc;
                }
            }
        }
        // 发送数据
				LCD_WriteData_Buffer(LCD_Buffer,buffer_size);
    }
    else  // 叠加方式
    {
        for(uint8_t i = 0; i < size; i++)  // 行循环
        {
            uint32_t char_data = 0;
						uint16_t byte_index = i * (size / 8);  // 动态计算起始索引
            
            // 获取字模数据
						if (size == 12 || size == 16)char_data = (data[i*2 + 1] << 8) | data[i*2];// 12/16像素：每行2字节
						else if (size == 24)char_data = (data[byte_index + 2] << 16) | (data[byte_index + 1] << 8) | data[byte_index];// 24像素：每行3字节（小端模式：低地址->低字节）
						else if (size == 32)char_data = (data[byte_index + 3] << 24) | (data[byte_index + 2] << 16) | (data[byte_index + 1] << 8) | data[byte_index];// 32像素：每行4字节（小端模式）
          
            // 处理一行中的所有像素
            for(uint8_t j = 0; j < size; j++) 
            {
                if(char_data & (1 << j)) {
                    LCD_DrawPoint(x + j, y + i, fc);  // 只绘制前景色像素
                }
            }
        }
    }
}

/**
  * @brief  显示中文字符串
  * @param  x: 起始X坐标
  * @param  y: 起始Y坐标
  * @param  fc: 前景色
  * @param  bc: 背景色
  * @param  Chinese: 中文字符串
  * @param  size: 字体大小
  * @param  mode: 显示模式
  * @retval 无
  */
void LCD_ShowChinese(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, const char *Chinese, uint8_t size, uint8_t mode) 
{
	char SigleChinese[4] = {0};
	uint8_t pChinese = 0;
	uint8_t pIndex;
		

			for (uint8_t i = 0; Chinese[i] != '\0'; i ++)
			{	
					SigleChinese[pChinese] = Chinese[i];
					pChinese ++;

					if (pChinese >= 3)
					{
						pChinese = 0;

						for (pIndex = 0; strcmp(chinese_font12[pIndex].Index, "") != 0; pIndex ++)
						{
							if (strcmp(chinese_font12[pIndex].Index, SigleChinese) == 0)
							{
								break;
							}
						}
						switch(size)
						{
						case 12:
							LCD_chinese_Buffer(x + ((i + 1) / 3 - 1) * size,y,POINT_COLOR,BACK_COLOR,(uint8_t *)chinese_font12[pIndex].data,size,mode);
							break;
						case 16:
							LCD_chinese_Buffer(x + ((i + 1) / 3 - 1) * size,y,POINT_COLOR,BACK_COLOR,(uint8_t *)chinese_font16[pIndex].data,size,mode);
							break;
						case 24:
							LCD_chinese_Buffer(x + ((i + 1) / 3 - 1) * size,y,POINT_COLOR,BACK_COLOR,(uint8_t *)chinese_font24[pIndex].data,size,mode);
							break;
						case 32:
							LCD_chinese_Buffer(x + ((i + 1) / 3 - 1) * size,y,POINT_COLOR,BACK_COLOR,(uint8_t *)chinese_font32[pIndex].data,size,mode);
							break;
						default:
							return;						
						}
					}

			}
	
}

/**
  * @brief  显示图片
  * @param  x: 起始X坐标
  * @param  y: 起始Y坐标
  * @param  Height: 图片高度
  * @param  Width: 图片宽度
  * @param  pic: 图片数据数组
  * @retval 无
  */
void LCD_ShowPicture(uint16_t x,uint16_t y,uint16_t Height,uint16_t Width,const uint8_t pic[])
{	
  LCD_SetWindows(x, y, x + Width - 1, y + Height - 1);
	LCD_WriteData_Buffer(pic,Height*Width*2);
			
}
