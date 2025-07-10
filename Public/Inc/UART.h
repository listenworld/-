/*
 * UART.h
 *
 *  Created on: Mar 10, 2025
 *      Author: lilang
 */

#ifndef INC_UART_H_
#define INC_UART_H_

#include "main.h"


extern UART_HandleTypeDef huart1;
//发送字符串
void Serial_SendString(char *String);
//发送数组
void Serial_SendArray(uint8_t *Array, uint16_t Length);
//vofa图示化
void Vofa_JustFloat_one(float data);
void Vofa_JustFloat_multiple(int num, ...);

#endif /* INC_UART_H_ */


