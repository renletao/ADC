/*
 * I2CHelper.h
 *
 *  Created on: 2020年8月4日
 *      Author: ITry
 */

#ifndef INC_I2CHELPER_H_
#define INC_I2CHELPER_H_
#include "SoftIIc.h"
#include "SoftIIc_PP.h"

typedef struct structI2CINFO {
	uint8_t bus_addr;
	uint8_t channel;
	uint8_t start_reg;//obsolete
	uint8_t data_lens;
} I2CInfo;

void I2C_IDLE(void);
uint8_t I2C_ReadNByte(I2CInfo info, uint16_t *readData);
uint8_t I2C_WriteNByte(I2CInfo info, const uint8_t *reg_data);
uint8_t WF2200_GetData( uint8_t *readData, uint8_t readNum);
uint8_t PP_WF2200_GetData( uint8_t *readData, uint8_t readNum);
#endif /* INC_I2CHELPER_H_ */
