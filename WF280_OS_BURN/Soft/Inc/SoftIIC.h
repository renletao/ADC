/*
 * SoftIIC.h
 *
 *  Created on: 2020年8月4日
 *      Author: ITry
 */

#ifndef INC_SOFTIIC_H_
#define INC_SOFTIIC_H_
#include "main.h"

#define IIC_WRITE 	0
#define IIC_READ 	1

typedef struct structI2CINFO {
	uint8_t bus_addr;
	uint8_t channel;
  uint8_t command;
	uint8_t data_lens;
} I2CInfo;

uint8_t I2C_ReadNByte(I2CInfo info, uint16_t *readData);
uint8_t I2C_WriteNByte(I2CInfo info, const uint8_t *reg_data);

void IIC_Init(GPIO_TypeDef *SDA_PORT, uint16_t SDA_PIN, GPIO_TypeDef *SCL_PORT, uint16_t SCL_PIN);
//void iic_init(GPIO_TypeDef *SDA_PORT, uint16_t SDA_PIN, GPIO_TypeDef *SCL_PORT, uint16_t SCL_PIN);
void iic_start(void);
void iic_stop(void);
void iic_ack(void);
void iic_nack(void);
void iic_sendbyte(uint8_t byte);
uint8_t iic_wait_ack(void);
uint8_t iic_readbyte(uint8_t ack);

#endif /* INC_SOFTIIC_H_ */
