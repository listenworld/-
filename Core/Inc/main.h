/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_RED_Pin GPIO_PIN_3
#define LED_RED_GPIO_Port GPIOE
#define LED_YELLOW_Pin GPIO_PIN_4
#define LED_YELLOW_GPIO_Port GPIOE
#define KEY_Pin GPIO_PIN_5
#define KEY_GPIO_Port GPIOE
#define ADC_V_Pin GPIO_PIN_2
#define ADC_V_GPIO_Port GPIOC
#define ADC_G_Pin GPIO_PIN_0
#define ADC_G_GPIO_Port GPIOA
#define ADC_I_Pin GPIO_PIN_1
#define ADC_I_GPIO_Port GPIOA
#define DC_I_Pin GPIO_PIN_2
#define DC_I_GPIO_Port GPIOA
#define DC_V_Pin GPIO_PIN_3
#define DC_V_GPIO_Port GPIOA
#define KEY3_Pin GPIO_PIN_13
#define KEY3_GPIO_Port GPIOD
#define KEY3_EXTI_IRQn EXTI15_10_IRQn
#define KEY2_Pin GPIO_PIN_14
#define KEY2_GPIO_Port GPIOD
#define KEY2_EXTI_IRQn EXTI15_10_IRQn
#define KEY1_Pin GPIO_PIN_15
#define KEY1_GPIO_Port GPIOD
#define KEY1_EXTI_IRQn EXTI15_10_IRQn
#define CS_Pin GPIO_PIN_1
#define CS_GPIO_Port GPIOD
#define TFT_RESET_Pin GPIO_PIN_2
#define TFT_RESET_GPIO_Port GPIOD
#define AK_Pin GPIO_PIN_3
#define AK_GPIO_Port GPIOD
#define RS_Pin GPIO_PIN_5
#define RS_GPIO_Port GPIOD
#define FLASH_SCL_Pin GPIO_PIN_0
#define FLASH_SCL_GPIO_Port GPIOE
#define FLASH_SDA_Pin GPIO_PIN_1
#define FLASH_SDA_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
