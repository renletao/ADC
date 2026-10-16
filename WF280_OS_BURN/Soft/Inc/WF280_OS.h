/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    OWI.h
  * @brief   This file contains all the function prototypes for
  *          the OWI.c file
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
#ifndef __WF280_OS_H__
#define __WF280_OS_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

/* USER CODE BEGIN Prototypes */
void ADC_MspInit(void);
void Get_ADC_Value(uint32_t *value);

uint8_t WF280_SetCmdOnly(uint8_t command, const uint8_t *para, uint8_t paraNum, uint8_t retryCount);
uint8_t WF280_SetCmdInNorMode(uint8_t command, const uint8_t *para, uint8_t paraNum, uint8_t delayMsBeforeStart, uint8_t retryCount);
uint8_t WF280_SetCmdInCmdMode(uint8_t command, const uint8_t *para, uint8_t paraNum, uint8_t delayMsBeforeStart, uint8_t retryCount);
uint8_t WF280_GetChipData(uint8_t command, uint8_t *readData, uint8_t readNum, uint8_t retryCount);
uint8_t WF280_GetData(uint8_t *readData, uint8_t readNum);
uint8_t WaitIdleInNor(uint8_t delayMsBeforeStart, uint8_t retryCount);
uint8_t WaitIdleInCmd(uint8_t delayMsBeforeStart, uint8_t retryCount);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif
#endif /*__OWI_H__ */

