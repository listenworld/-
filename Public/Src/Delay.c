/*
 * Delay.c
 *
 *  Created on: Mar 17, 2025
 *      Author: lilang
 */


/***********************************
	本驱动文件仅适配HAL库版本
***********************************/
#include "stm32g4xx_hal.h"
#include "Delay.h"

/**
  * @brief  微秒级延时函数（HAL库）
  * @param  xus 延时的微秒数（范围：1~786432）
  * @note   基于SysTick实现，不影响HAL_Delay()
  */
void Delay_us(uint32_t xus)
{
    uint32_t temp;
    
    // 临时关闭SysTick中断（防止与HAL_Delay冲突）
    SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk;  
    
    // 配置SysTick：170MHz → 1μs = 170 cycles
    SysTick->LOAD = 170 * xus - 1;     					// 重载值 = 170*xus -1 （24位寄存器最大0xFFFFFF）
    SysTick->VAL = 0;                  					// 清空计数器
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;  	// 启动SysTick（使用HCLK，不使能中断）
    
    // 等待计数完成
    do {
        temp = SysTick->CTRL;
    } while (!(temp & SysTick_CTRL_COUNTFLAG_Msk));
    
    // 关闭SysTick并恢复中断设置
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
    SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;  
}

/**
  * @brief  毫秒级延时
  * @param  xms 延时的毫秒数（范围：1~4294967）
  */
void Delay_ms(uint32_t xms) 
{
	while(xms--)
	{
		Delay_us(1000);
	}
}

/**
  * @brief  秒级延时
  * @param  xs 延时的秒数（范围：1~4294967）
  */
void Delay_s(uint32_t xs) 
{
    while(xs--) {
        Delay_ms(1000);  // 1秒 = 1000毫秒
    }
}


