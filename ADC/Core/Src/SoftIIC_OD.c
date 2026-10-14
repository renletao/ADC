/*
 * SoftIIC.c
 *
 *  Created on: Aug 3, 2020
 *      Author: ITry
 */
#include "SoftIIC.h"

GPIO_TypeDef *portSda = SCL_GPIO_Port;
GPIO_TypeDef *portScl = SDA_GPIO_Port;
uint16_t pinSda = SDA_Pin;
uint16_t pinScl = SCL_Pin;

//设置高低电平
#define SetSoftI2CSDA() HAL_GPIO_WritePin(portSda, pinSda, GPIO_PIN_SET)
#define ClrSoftI2CSDA() HAL_GPIO_WritePin(portSda, pinSda, GPIO_PIN_RESET)
#define SetSoftI2CSCL() HAL_GPIO_WritePin(portScl, pinScl, GPIO_PIN_SET)
#define ClrSoftI2CSCL() HAL_GPIO_WritePin(portScl, pinScl, GPIO_PIN_RESET)
//读取高低电平
#define SoftI2C_SDA_H() (HAL_GPIO_ReadPin(portSda, pinSda) == GPIO_PIN_SET)
#define SoftI2C_SDA_L() (HAL_GPIO_ReadPin(portSda, pinSda) == GPIO_PIN_RESET)
#define SoftI2C_SCL_H() (HAL_GPIO_ReadPin(portScl, pinScl) == GPIO_PIN_SET)

void SoftI2CSclIn(void)
{
  //OD
}

void SoftI2CSclOut(void)
{
  //OD
}

void SoftI2CSdaIn(void)
{
    //OD
  SetSoftI2CSDA();
}

void SoftI2CSdaOut(void)
{
	//OD
}

void iic_init(GPIO_TypeDef *SDA_PORT, uint16_t SDA_PIN, GPIO_TypeDef *SCL_PORT, uint16_t SCL_PIN)
{
	portSda = SDA_PORT;
	portScl = SCL_PORT;
	pinSda = SDA_PIN;
	pinScl = SCL_PIN;

	SoftI2CSclOut();
	SoftI2CSdaOut();
}


/**
 *iic启动:当SCL处于高电平状态时，SDA出现一个下降沿,即产生IIC启动信号
 */
void iic_start(void) {
	SoftI2CSclOut();
	SoftI2CSdaOut();
	SetSoftI2CSDA();
	SetSoftI2CSCL();
	delay_us(20);
	ClrSoftI2CSDA();
	delay_us(20);
	ClrSoftI2CSCL();
  delay_us(10);
}

/**
 *iic停止:当SCL处于高电平状态时，SDA出现一个上升沿,即产生IIC停止信号
 */
void iic_stop(void) {
	SoftI2CSdaOut();
	ClrSoftI2CSCL();
	ClrSoftI2CSDA();
	delay_us(20);
	SetSoftI2CSCL();
	SetSoftI2CSDA();
	delay_us(20);
}

/**
 *IIC等待应答信号
 *ack_status： 应答状态，0表示应答，1表示设备无响应
 */
uint8_t iic_wait_ack(void) {
	uint8_t err_time = 0;
	SoftI2CSdaIn();
	SetSoftI2CSDA();
	delay_us(20);
	SetSoftI2CSCL();
	delay_us(20);
	while (SoftI2C_SDA_H()) {
		err_time++;
		if (err_time > 250) {
		//	iic_stop();
			return 1;
		}
	}
	ClrSoftI2CSCL();
	return 0;
}

/**
 *主机（主控制器）产生应答信号
 */
void iic_ack(void) {
	ClrSoftI2CSCL();
	SoftI2CSdaOut();
	ClrSoftI2CSDA();
	delay_us(20);
	SetSoftI2CSCL();
	delay_us(20);
	ClrSoftI2CSCL();
}

/**
 * 主机（主控制器）产生不应答信号
 */
void iic_nack(void) {
	ClrSoftI2CSCL();
	SoftI2CSdaOut();
	SetSoftI2CSDA();
	delay_us(20);
	SetSoftI2CSCL();
	delay_us(20);
	ClrSoftI2CSCL();
}

/**
 *IIC发送1byte数据
 */
void iic_sendbyte(uint8_t byte) {
	uint8_t i;
	SoftI2CSdaOut();
	ClrSoftI2CSCL();
	/** 发送一个字节的高7位 */
	for (i = 0; i < 8; i++) {
		if (byte & 0x80) {
			SetSoftI2CSDA();
		} else {
			ClrSoftI2CSDA();
		}
		byte <<= 1;

		SetSoftI2CSCL();
		delay_us(20);
		ClrSoftI2CSCL();
		delay_us(20);
	}
}

/**
 *IIC读取1byte数据
 */
uint8_t iic_readbyte(uint8_t ack) {
	uint8_t i;
	uint8_t recv_value = 0;
	SoftI2CSdaIn();
	for (i = 0; i < 8; i++) {
		ClrSoftI2CSCL();
		delay_us(20);
		SetSoftI2CSCL();
		delay_us(10);
		recv_value <<= 1;
		if (SoftI2C_SDA_H()) {
			recv_value++;
		}
    delay_us(10);
	}
	if(!ack){
		iic_nack();
	}
	else{
		iic_ack();
	}
	return recv_value;
}

void iic_idle(void){
    //OD
}
