/*
 * EEPROM_M24C02.c
 *
 *  Created on: Mar 17, 2025
 *      Author: lilang
 */


/***********************************
	本驱动文件仅适配HAL库版本
	note:需要先配置好对应的GPIO引脚
***********************************/
#include "stm32g4xx_hal.h"
#include "EEPROM_M24C02.h"
#include "Delay.h"

/*------------------------------------------I2C---------------------------------------------------*/

/**
  * @brief  I2C时钟线(SCL)电平控制
  * @param  Bit 电平状态 (0:低电平, 1:高电平)
  * @note   操作后固定延时10μs保证时序
  */
static void I2C_W_SCL(uint8_t Bit)
{
	HAL_GPIO_WritePin(M24C02_SCL_PORT,M24C02_SCL_PIN,(Bit ? GPIO_PIN_SET : GPIO_PIN_RESET));
	Delay_us(10);
}

/**
  * @brief  I2C数据线(SDA)电平控制  
  * @param  Bit 电平状态 (0:低电平, 1:高电平)
  * @note   操作后固定延时10μs保证时序
  */
static void I2C_W_SDA(uint8_t Bit)
{
	HAL_GPIO_WritePin(M24C02_SDA_PORT,M24C02_SDA_PIN,(Bit ? GPIO_PIN_SET : GPIO_PIN_RESET));
	Delay_us(10);
}

/**
  * @brief  I2C数据线(SDA)电平读取
  * @retval 当前SDA电平状态 (GPIO_PIN_SET或GPIO_PIN_RESET)
  * @note   读取后固定延时10μs保证时序
  */
static uint8_t I2C_R_SDA(void)
{
	uint8_t Bit;
	Bit=HAL_GPIO_ReadPin(M24C02_SDA_PORT,M24C02_SDA_PIN);
	Delay_us(10);
	return Bit;
}

/**
  * @brief  产生I2C起始信号
  * @note   时序：SDA高→SCL高→SDA低→SCL低
  */
static void I2C_Start(void)
{
	I2C_W_SDA(1);
	I2C_W_SCL(1);
	I2C_W_SDA(0);
	I2C_W_SCL(0);
}

/**
  * @brief  产生I2C停止信号
  * @note   时序：SDA低→SCL高→SDA高
  */
static void I2C_Stop(void)
{
	I2C_W_SDA(0);
	I2C_W_SCL(1);
	I2C_W_SDA(1);
}

/**
  * @brief  I2C发送单个字节
  * @param  Byte 要发送的数据字节（MSB先行）
  * @note   每个数据位在SCL高电平期间保持稳定
  */
static void I2C_SendByte(uint8_t Byte)//发送
{
	uint8_t i;
	for(i=0;i<8;i++)
	{
		I2C_W_SDA(Byte&(0x80>>i));
		I2C_W_SCL(1);
		I2C_W_SCL(0);
	}
}

/**
  * @brief  I2C接收单个字节
  * @retval 接收到的数据字节
  * @note   接收前需释放SDA线（置高电平）
  */
static uint8_t I2C_ReceiveByte(void)//接收
{
	uint8_t i,Byte=0x00;
	I2C_W_SDA(1);
	for(i=0;i<8;i++)
	{
		I2C_W_SCL(1);
		if(I2C_R_SDA()==1)
		{
			Byte|=(0x80>>i);
		}
		I2C_W_SCL(0);
	}
	return Byte;
}

/**
  * @brief  I2C发送应答信号
  * @param  AckBit 应答类型 (0:ACK, 1:NACK)
  * @note   应在第9个时钟周期内完成应答
  */
static void I2C_SendAck(uint8_t AckBit)//发送应答
{
		I2C_W_SDA(AckBit);
		I2C_W_SCL(1);
		I2C_W_SCL(0);
}

/**
  * @brief  I2C等待应答信号
  * @retval 0:收到ACK, 1:超时未收到ACK
  * @note   设置1000次循环超时检测（约10ms@170MHz）
  */
static uint8_t I2C_WaitAck(void)//接收应答
{
	uint16_t Timeout_cnt=1000;
	
	I2C_W_SDA(1);
	I2C_W_SCL(1);	
	while(I2C_R_SDA())//读取SDA，如果为0，则跳出循环
	{
			Timeout_cnt--;
			if (0 == Timeout_cnt)//错误处理
			{
			
				break;
			}
	}
	I2C_W_SCL(0);
	
	return 0;//表示返回成功
}

/*------------------------------------------M24C02---------------------------------------------------*/

/**
  * @brief  向M24C02写入单个字节
  * @param  Addr 写入地址（0x00~0xFF）
  * @param  data 要写入的数据字节
  * @note   写入后固定延时6ms等待内部编程周期
  */
void M24C02_Write_Byte(uint8_t Addr, uint8_t data)
{
	I2C_Start();
	I2C_SendByte(M24C02_ADDRESS);
	I2C_WaitAck();
	I2C_SendByte(Addr);
	I2C_WaitAck();
	I2C_SendByte(data);
	I2C_WaitAck();
	I2C_Stop();
	Delay_ms(6);
}

/**
  * @brief  向M24C02页写入数据
  * @param  Addr 起始地址（0x00~0xFF）
  * @param  data 数据缓冲区指针
  * @param  Length 数据长度（1~16字节）
  * @note   地址自动对齐页边界，跨页写入将被截断
  */
void M24C02_Write_Page(uint8_t Addr, uint8_t *data,uint8_t Length)
{
	if(Length == 0 || Length > M24C02_PAGE_SIZE) return;		//一页最多写入16个字节

	uint8_t i;
	I2C_Start();
	I2C_SendByte(M24C02_ADDRESS);
	I2C_WaitAck();
	I2C_SendByte(Addr);
	I2C_WaitAck();
	for(i=0;i<Length;i++)
	{		
		I2C_SendByte(data[i]);
		I2C_WaitAck();		
	}

	I2C_Stop();
	Delay_ms(6);
}

/**
  * @brief  从M24C02读取单个字节
  * @param  Addr 读取地址（0x00~0xFF）
  * @retval 读取到的数据字节
  */
uint8_t M24C02_Read_Byte(uint8_t Addr) 
{
  uint8_t data = 0;

	I2C_Start();
	I2C_SendByte(M24C02_ADDRESS);
	I2C_SendAck(0);
	I2C_SendByte(Addr);
	I2C_SendAck(0);

	I2C_Start();
	I2C_SendByte(M24C02_ADDRESS|0x01);
	I2C_SendAck(0);
	data = I2C_ReceiveByte();
	I2C_Stop();
	
	return data;
}

/**
  * @brief  从M24C02连续读取多个字节
  * @param  Addr 起始地址（0x00~0xFF）
  * @param  data 数据缓冲区指针
  * @param  Length 要读取的字节数（1~256）
  * @note   支持跨页连续读取
  */
void M24C02_Read_Multi_Byte(uint8_t Addr,uint8_t *data,uint8_t Length) 
{
  uint8_t i;

	I2C_Start();
	I2C_SendByte(M24C02_ADDRESS);
	I2C_SendAck(0);
	I2C_SendByte(Addr);
	I2C_SendAck(0);

	I2C_Start();
	I2C_SendByte(M24C02_ADDRESS|0x01);
	for(i=0;i<Length;i++)
	{
		I2C_SendAck(0);
		data[i] = I2C_ReceiveByte();		
	}
	I2C_Stop();
}

