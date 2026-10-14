/*
 * CmdHelper.h
 *
 *  Created on: Feb 17, 2022
 *      Author: ITry
 */

#ifndef INC_CMDHELPER_H_
#define INC_CMDHELPER_H_

#include "stm32g0xx_hal.h"

#define TYPE1_GET_VERSION 					0x104
#define TYPE1_GET_ADC	 							0x105
#define TYPE1_SET_VDD								0x201
#define TYPE1_SET_ADC								0x107
#define TYPE1_READ_ADC							0x108
#define TYPE1_CALIB									0x109
#define TYPE1_SAVE_ADC							0x200
#define TYPE1_GET_TASK_STATUS 			0x400
#define TYPE1_GET_ALLTEST_DATA      0x401
#define TYPE1_WF3050_FT_DATA	      0x567

#define WF183DE 0
#define WF2200	1
#define WF100S  2

#define READ_ADC  0
#define READ_IIC  1
#define READ_UART 2
#define READ_OWI  3

typedef struct WF183D_Device
{
	uint8_t Uart_State;
	uint16_t BaudRate;
	uint8_t Uart_SendBuff[4];
	uint8_t Uart_RevBuff[8];
	uint8_t I2C_State;
	uint8_t Owi_State;
	uint8_t Owi_Buff[3];
	uint8_t Owi_BaudRate;
} WF183D_Dev;

uint8_t SetProductVpp(uint8_t dp);
void Parse(uint16_t* cmd, uint8_t cmd_lens, uint16_t* out, uint16_t out_lens);
void ParseSystemCmd(uint16_t *cmd, uint8_t cmd_lens, uint16_t *out, uint16_t out_lens);
void CopyAndClear(uint16_t* src, uint16_t* dest, size_t len);
void ParseI2CCmd(uint16_t *cmd, uint8_t cmd_lens, uint16_t *out, uint16_t out_lens);
void ParseSerialCmd(uint16_t *cmd, uint8_t cmd_lens, uint16_t *out, uint16_t out_lens);
void ParseProductCmd(uint16_t *cmd, uint8_t cmd_lens, uint16_t *out, uint16_t out_lens);
#endif /* INC_CMDHELPER_H_ */
