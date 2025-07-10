/*
 * EEPROM_M24C02.h
 *
 *  Created on: Mar 17, 2025
 *      Author: lilang
 */

#ifndef INC_EEPROM_M24C02_H_
#define INC_EEPROM_M24C02_H_

#include "main.h"

/*----------------I2C宏定义----------------*/
#define			M24C02_SCL_PORT				GPIOE
#define			M24C02_SDA_PORT				GPIOE
#define			M24C02_SCL_PIN				GPIO_PIN_0
#define			M24C02_SDA_PIN				GPIO_PIN_1

/*----------------M24C02宏定义----------------*/
#define			M24C02_ADDRESS				0xA0							//1010 000 0
#define			M24C02_PAGE_SIZE			16								//一页16个字节





void M24C02_Write_Byte(uint8_t Addr, uint8_t data);
void M24C02_Write_Page(uint8_t Addr, uint8_t *data,uint8_t Length);
uint8_t M24C02_Read_Byte(uint8_t Addr);
void M24C02_Read_Multi_Byte(uint8_t Addr,uint8_t *data,uint8_t Length);












#endif /* INC_EEPROM_M24C02_H_ */


