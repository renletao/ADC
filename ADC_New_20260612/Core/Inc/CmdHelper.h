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
#define TYPE1_SET_VPP							  0x202
#define TYPE1_SET_ADC								0x107
#define TYPE1_READ_ADC							0x108
#define TYPE1_CALIB									0x109
#define TYPE1_SAVE_ADC							0x200

void Parse(uint16_t* cmd, uint8_t cmd_lens, uint16_t* out, uint16_t out_lens);
void ParseSystemCmd(uint16_t *cmd, uint8_t cmd_lens, uint16_t *out, uint16_t out_lens);
void CopyAndClear(uint16_t* src, uint16_t* dest, size_t len);
void ParseI2CCmd(uint16_t *cmd, uint8_t cmd_lens, uint16_t *out, uint16_t out_lens);
void ParseSerialCmd(uint16_t *cmd, uint8_t cmd_lens, uint16_t *out, uint16_t out_lens);
void ParseProductCmd(uint16_t *cmd, uint8_t cmd_lens, uint16_t *out, uint16_t out_lens);
#endif /* INC_CMDHELPER_H_ */
