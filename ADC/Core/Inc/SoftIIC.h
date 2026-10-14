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

void iic_init(GPIO_TypeDef *SDA_PORT, uint16_t SDA_PIN, GPIO_TypeDef *SCL_PORT, uint16_t SCL_PIN);
void iic_start(void);
void iic_stop(void);
void iic_ack(void);
void iic_nack(void);
void iic_sendbyte(uint8_t byte);
uint8_t iic_wait_ack(void);
uint8_t iic_readbyte(uint8_t ack);
void iic_idle(void);


#endif /* INC_SOFTIIC_H_ */
