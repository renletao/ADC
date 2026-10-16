/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "stm32g0xx_hal.h"

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
void delay_us(uint32_t us);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define GND_ADC_Pin GPIO_PIN_0
#define GND_ADC_GPIO_Port GPIOA
#define PULL_UP_Pin GPIO_PIN_1
#define PULL_UP_GPIO_Port GPIOA
#define USART_TX_Pin GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA
#define USART_RX_Pin GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA
#define VDD_ADC_Pin GPIO_PIN_4
#define VDD_ADC_GPIO_Port GPIOA
#define VPP_ADC_Pin GPIO_PIN_5
#define VPP_ADC_GPIO_Port GPIOA
#define IIC_SCL_Pin GPIO_PIN_6
#define IIC_SCL_GPIO_Port GPIOA
#define IIC_SDA_Pin GPIO_PIN_7
#define IIC_SDA_GPIO_Port GPIOA
#define OWI_Pin GPIO_PIN_0
#define OWI_GPIO_Port GPIOB
#define VPP_EN_Pin GPIO_PIN_1
#define VPP_EN_GPIO_Port GPIOB
#define M1_EN_Pin GPIO_PIN_8
#define M1_EN_GPIO_Port GPIOA
#define VDD_EN_Pin GPIO_PIN_6
#define VDD_EN_GPIO_Port GPIOC
#define OS_EN_Pin GPIO_PIN_12
#define OS_EN_GPIO_Port GPIOA
#define LED_Pin GPIO_PIN_8
#define LED_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
