/*
 * UART.c
 *
 *  Created on: Mar  10, 2025
 *      Founder: lilang
 *
 *	revised in: April 1, 2025
 *		Reviser: Qxb
 */


/***********************************
	本驱动文件仅适配HAL库版本
***********************************/
#include "stm32g4xx_hal.h"
#include "UART.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

/**************************************************************
* 函    数：串口发送一个字节
* 参    数：Byte 要发送的一个字节
* 返 回 值：无
***************************************************************/
void Serial_SendByte(uint8_t Byte)
{
		HAL_UART_Transmit(&huart1, &Byte ,1,10);
}

/**************************************************************
* 函    数：串口发送字符串
* 参    数：String 要发送字符串
* 返 回 值：无
***************************************************************/
void Serial_SendString(char *String)
{
		HAL_UART_Transmit(&huart1,(uint8_t *)String,strlen(String),10);
}

/**************************************************************
* 函    数：串口发送数组
* 参    数：Array 要发送数组
* 返 回 值：无
***************************************************************/
void Serial_SendArray(uint8_t *Array, uint16_t Length)
{
		HAL_UART_Transmit(&huart1,Array,Length,10);
}

/***************************************************************************************************** 
* 函	数: 	JustFloat_Vofa_one
* 输入参数: float data - 单个待发送数据  
* 功 能: VOFA 调试助手数据发送函数，用于将单通道数据发送到 VOFA 进行实时监测  
******************************************************************************************************/ 
void Vofa_JustFloat_one(float data)
{
    // 定义尾帧结构
    struct Frame {
        float fdata[1];
        unsigned char tail[4];
    };

    struct Frame frame = {
        .fdata = {0},
        .tail = {0x00, 0x00, 0x80, 0x7f}
    };
    // 填充浮点数数组
    frame.fdata[0] =  data ;
    // 发送浮点数数组
    Serial_SendArray((uint8_t *)frame.fdata, sizeof(frame.fdata));
    // 发送帧尾
    Serial_SendArray(frame.tail, sizeof(frame.tail));
}
/***************************************************************************************************** 
* 函	数:   JustFloat_vofa_multiple
* 输入参数: float data - 多个待发送数据
* 	num	  ：传入数据数量建议使用七个以内
* 功 能: VOFA 调试助手数据发送函数，用于将多通道数据发送到 VOFA 进行实时监测  
******************************************************************************************************/ 
void Vofa_JustFloat_multiple(int num, ...)
{
	va_list link;
	//从num存储数据
	va_start(link, num);
    // 定义尾帧结构
    struct {
        float fdata[8];//发送数据个数上限，可修改
        unsigned char tail[4];
    } 	frame = { {0}, {0x00, 0x00, 0x80, 0x7f} };

	for (uint8_t i = 0; i < num; i++)
	{	// 不断取出可变参数
		frame.fdata[i] = (float)va_arg(link, double );
	}
    // 发送浮点数数组
    Serial_SendArray((uint8_t *)frame.fdata, sizeof(frame.fdata));
    // 发送帧尾
    Serial_SendArray(frame.tail, sizeof(frame.tail));
	// 销毁数据
	va_end(link);
}


