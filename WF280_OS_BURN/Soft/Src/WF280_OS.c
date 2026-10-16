/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
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

/* Includes ------------------------------------------------------------------*/
#include "WF280_OS.h"


/* USER CODE BEGIN 0 */
#include "SoftIIC.h"
#include "cmsis_os.h"
#include "string.h"
#include "adc.h"

#define WF280IIC_ADDRESS 0x70
/* USER CODE END 0 */
uint8_t ADC_State = 0;
/* USER CODE BEGIN 1 */

void ADC_MspInit()
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = GND_ADC_Pin|USART_TX_Pin|USART_RX_Pin|VDD_ADC_Pin|VPP_ADC_Pin|IIC_SCL_Pin|IIC_SDA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	GPIO_InitStruct.Pin = OWI_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(OWI_GPIO_Port, &GPIO_InitStruct);  
}

void Get_ADC_Value(uint32_t* value)
{      
  uint32_t count = 0;
  HAL_ADC_Start_DMA(&hadc1,value,8); 
  while(!ADC_State)
  {
    count++;
    ADC_State = 0;
    osDelay(1);
    if(count>=100)return;
  }  
  HAL_ADC_Start_DMA(&hadc1,value,8); 
  while(!ADC_State)
  {
    count++;
    ADC_State = 0;
    osDelay(1);
    if(count>=100)return;
  }
  
}
/* USER CODE BEGIN 2 */

uint8_t I2C_WriteBytes(uint8_t command, const uint8_t *para, uint8_t paraNum) {
	uint8_t i = 0;
	uint8_t err = 0;
	do {
		iic_start();
		iic_sendbyte(WF280IIC_ADDRESS | IIC_WRITE);
		if (iic_wait_ack() != 0){
			err = 1;
			break;
		}

		iic_sendbyte(command);
		if (iic_wait_ack() != 0){
			err = 1;
			break;
		}

		for (i = 0; i < paraNum; i++) {
			iic_sendbyte(para[i]);
			if (iic_wait_ack() != 0){
				err = 1;
				break;
			}
		}

	} while (0);
	iic_stop();
	//iic_idle();
	return err;
}
uint8_t I2C_ReadBytes( uint8_t *readData, uint8_t readNum) {
	uint8_t i = 0;
	uint8_t err = 0;
	do {
		iic_start();
		iic_sendbyte(WF280IIC_ADDRESS | IIC_READ);
		if (iic_wait_ack() != 0){
			err = 1;
			break;
		}

		for (i = 0; i < readNum; i++) {
			readData[i] = 0;
			if (i != (readNum - 1)) {
				readData[i] = iic_readbyte(1);
			} else {
				readData[i] = iic_readbyte(0);
			}
		}

	} while (0);
	iic_stop();
	//iic_idle();
	return err;
}

//
//bit7必须为0，bit0必须为1
//
uint8_t WaitIdleInNor(uint8_t delayMsBeforeStart, uint8_t retryCount) {
	uint8_t i = 0;
	uint8_t err = 0;
	static uint8_t buf[1];
	if(delayMsBeforeStart > 0) osDelay(delayMsBeforeStart);
	for (i = 0; i < retryCount; i++) {
		buf[0] = 0;
		if((err = I2C_ReadBytes(buf ,1)) != 0) break;
		if((buf[0] & 0x80) == 0 &&  (buf[0] & 0x01) == 0x01) break;
		err = 2;
		osDelay(10);
	}
	return err;
}

//
//bit7必须为0，bit6必须为1，bit0必须为1
//
uint8_t WaitIdleInCmd( uint8_t delayMsBeforeStart, uint8_t retryCount) {
	uint8_t i = 0;
	uint8_t err = 0;
	static uint8_t buf[1];
	osDelay(delayMsBeforeStart);
	for (i = 0; i < retryCount; i++) {
		buf[0] = 0;
		if((err = I2C_ReadBytes(buf ,1)) != 0) break;
		if((buf[0] & 0x80) == 0 &&  (buf[0] & 0x41) == 0x41) break;
		err = 3;
		osDelay(1);
	}
	return err;
}

uint8_t WF280_SetCmdOnly(uint8_t command, const uint8_t *para, uint8_t paraNum, uint8_t retryCount) {
	uint8_t err = 0;
	err = I2C_WriteBytes( command, para, paraNum);
	return err;
}

uint8_t WF280_SetCmdInNorMode( uint8_t command, const uint8_t *para, uint8_t paraNum, uint8_t delayMsBeforeStart, uint8_t retryCount) {
	uint8_t err = 0;
	do {
		if ((err = I2C_WriteBytes( command, para, paraNum)) != 0)
			break;

		if ((err = WaitIdleInNor( delayMsBeforeStart, retryCount)) != 0)
			break;
	} while (0);
	return err;
}

uint8_t WF280_SetCmdInCmdMode(uint8_t command, const uint8_t *para, uint8_t paraNum, uint8_t delayMsBeforeStart, uint8_t retryCount) {
	uint8_t err = 0;
	do {
		if ((err = I2C_WriteBytes( command, para, paraNum)) != 0)
			break;

		if ((err = WaitIdleInCmd( delayMsBeforeStart, retryCount)) != 0)
			break;
	} while (0);
	return err;
}

uint8_t WF280_GetChipData( uint8_t command, uint8_t *readData, uint8_t readNum, uint8_t retryCount) {
	uint8_t err = 0;
	do {
		if(readNum > 0) memset(readData, 0, (readNum * sizeof(uint8_t)));
		if ((err = I2C_WriteBytes( command, NULL, 0)) != 0)
			break;
		if ((err = WaitIdleInNor( 0, retryCount)) != 0)
			break;
		if ((err = I2C_ReadBytes(readData, readNum)) != 0)
			break;
	} while (0);
	return err;
}

uint8_t WF280_GetData( uint8_t *readData, uint8_t readNum) {
	uint8_t err = 0;
	if(readNum > 0) memset(readData, 0, (readNum * sizeof(uint8_t)));
	err = I2C_ReadBytes( readData, readNum);
	return err;
}


/* USER CODE END 2 */

 void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
 {
   ADC_State = 1;
   HAL_ADC_Stop_DMA(&hadc1);
 }



