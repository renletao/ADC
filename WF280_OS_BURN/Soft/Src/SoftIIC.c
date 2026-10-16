/*
 * SoftIIC.c
 *
 *  Created on: Aug 3, 2020
 *      Author: ITry
 */
#include "SoftIIC.h"
#include <string.h>
#include "delay.h"

GPIO_TypeDef *portSda = IIC_SDA_GPIO_Port;
GPIO_TypeDef *portScl = IIC_SCL_GPIO_Port;
uint16_t pinSda = IIC_SDA_Pin;
uint16_t pinScl = IIC_SCL_Pin;

//设置高低电平
#define I2CSDA_HIGH() HAL_GPIO_WritePin(portSda, pinSda, GPIO_PIN_SET)
#define I2CSDA_LOW() HAL_GPIO_WritePin(portSda, pinSda, GPIO_PIN_RESET)
#define I2CSCL_HIGH() HAL_GPIO_WritePin(portScl, pinScl, GPIO_PIN_SET)
#define I2CSCL_LOW() HAL_GPIO_WritePin(portScl, pinScl, GPIO_PIN_RESET)
//读取高低电平
#define SoftI2C_SDA_H() (HAL_GPIO_ReadPin(portSda, pinSda) == GPIO_PIN_SET)
#define SoftI2C_SDA_L() (HAL_GPIO_ReadPin(portSda, pinSda) == GPIO_PIN_RESET)
#define SoftI2C_SCL_H() (HAL_GPIO_ReadPin(portScl, pinScl) == GPIO_PIN_SET)


/**
 *IO初始化
 */
void IIC_MspInit(GPIO_TypeDef *SDA_PORT, uint16_t SDA_PIN, GPIO_TypeDef *SCL_PORT, uint16_t SCL_PIN)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	
	HAL_GPIO_WritePin(SDA_PORT, SDA_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(SCL_PORT, SCL_PIN, GPIO_PIN_SET);
	  
	GPIO_InitStruct.Pin = SDA_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(SDA_PORT, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin = SCL_PIN;
  HAL_GPIO_Init(SCL_PORT, &GPIO_InitStruct);
}

/**
 *IO取消初始化
 */
void IIC_DeMspInit(GPIO_TypeDef *SDA_PORT, uint16_t SDA_PIN, GPIO_TypeDef *SCL_PORT, uint16_t SCL_PIN)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	
	HAL_GPIO_WritePin(SDA_PORT, SDA_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(SCL_PORT, SCL_PIN, GPIO_PIN_RESET);
	  
	GPIO_InitStruct.Pin = SDA_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(SDA_PORT, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin = SCL_PIN;
  HAL_GPIO_Init(SCL_PORT, &GPIO_InitStruct);
}
/**
 *iic初始化
 */
void IIC_Init(GPIO_TypeDef *SDA_PORT, uint16_t SDA_PIN, GPIO_TypeDef *SCL_PORT, uint16_t SCL_PIN)
{
	portSda = SDA_PORT;
	portScl = SCL_PORT;
	pinSda = SDA_PIN;
	pinScl = SCL_PIN;
	IIC_MspInit(portSda,pinSda,portScl,pinScl);
  HAL_GPIO_WritePin(USART_TX_GPIO_Port, USART_TX_Pin, GPIO_PIN_SET);
}

/**
 *iic取消初始化
 */
void IIC_DeInit(GPIO_TypeDef *SDA_PORT, uint16_t SDA_PIN, GPIO_TypeDef *SCL_PORT, uint16_t SCL_PIN)
{
	IIC_DeMspInit(portSda,pinSda,portScl,pinScl);
	portSda = NULL;
	portScl = NULL;
	pinSda = NULL;
	pinScl = NULL;
}

/**
 *iic启动:当SCL处于高电平状态时，SDA出现一个下降沿,即产生IIC启动信号
 */
void iic_start(void)
{
	I2CSDA_HIGH();
	I2CSCL_HIGH();
	delay_us(20);
	I2CSDA_LOW();
	delay_us(20);
	I2CSCL_LOW();
  delay_us(20);
}

/**
 *iic停止:当SCL处于高电平状态时，SDA出现一个上升沿,即产生IIC停止信号
 */
void iic_stop(void) 
{
	I2CSDA_LOW();
	I2CSCL_HIGH();
	delay_us(20);
	I2CSDA_HIGH();
	delay_us(20);
}

/**
 *IIC等待应答信号
 *ack_status： 应答状态，0表示应答，1表示设备无响应
 */
uint8_t iic_wait_ack(void) 
{
	uint8_t err_time = 0;
	I2CSDA_HIGH();
	delay_us(20);
	I2CSCL_HIGH();
	delay_us(20);
	while (SoftI2C_SDA_H())
	{
		err_time++;
		if (err_time > 250)
		{
			//iic_stop();
			return 1;
		}
	}
	I2CSCL_LOW();
	delay_us(20);
	return 0;
}

/**
 *主机（主控制器）产生应答信号
 */
void iic_ack(void)
{
	I2CSDA_LOW();
	delay_us(20);
	I2CSCL_HIGH();
	delay_us(20);
	I2CSCL_LOW();
	delay_us(20);
	I2CSDA_HIGH();
}

/**
 * 主机（主控制器）产生不应答信号
 */
void iic_nack(void) 
{
	I2CSDA_HIGH();
	delay_us(20);
	I2CSCL_HIGH();
	delay_us(20);
	I2CSCL_LOW();
	delay_us(20);
}

/**
 *IIC发送1byte数据
 */
void iic_sendbyte(uint8_t byte) 
{
	uint8_t i;
	/** 发送一个字节的高7位 */
	for (i = 0; i < 8; i++) 
	{
		if (byte & 0x80) 
		{
			I2CSDA_HIGH();
		}
		else
		{
			I2CSDA_LOW();
		}
		byte <<= 1;
		delay_us(10);
		I2CSCL_HIGH();
		delay_us(20);
		I2CSCL_LOW();
		delay_us(10);
	}
}

/**
 *IIC读取1byte数据
 */
uint8_t iic_readbyte(uint8_t ack)
{
	uint8_t i;
	uint8_t recv_value = 0;
	I2CSDA_HIGH();
  delay_us(20);
	for (i = 0; i < 8; i++) 
	{
		I2CSCL_HIGH();
		delay_us(10);		
    recv_value <<= 1;
		if(SoftI2C_SDA_H()) 
		{
			recv_value++;
		}
    delay_us(10);		
		I2CSCL_LOW();
		delay_us(20);
	}
	if(!ack)
	{
		iic_nack();
	}
	else
	{
		iic_ack();
	}
	return recv_value;
}
/**
 *iic多字节读取
 */
uint8_t I2C_ReadNByte(I2CInfo info, uint16_t *readData) 
{
	uint8_t i = 0;
	uint8_t err = 0;
	if(info.data_lens > 0) memset(readData, 0, (info.data_lens * sizeof(uint8_t)));
		
	iic_start();
		iic_sendbyte(info.bus_addr | IIC_READ);
		err = iic_wait_ack();
		if (err == 0) {
			for (i = 0; i < info.data_lens; i++) {
				if (i != (info.data_lens - 1)) {
					readData[i] = iic_readbyte(1); // 璇诲畬闈炴渶鍚庝竴涓瓧鑺傚悗锛� 浜х敓搴旂瓟淇″彿
				} else {
					readData[i] = iic_readbyte(0); // 鏈�鍚庝竴涓瓧鑺傝瀹屽悗 浜х敓闈炲簲绛斾俊鍙�,浠ユ彁绀篠lave璁惧鍑嗗鎺ユ敹Stop淇″彿
				}
			}
		}
		iic_stop();
    
	return err;
}
/**
 *iic多字节发送
 */
uint8_t I2C_WriteNByte(I2CInfo info, const uint8_t *writeData) 
{
	uint8_t i = 0;
	uint8_t err = 0;
	iic_start();
		iic_sendbyte(info.bus_addr | IIC_WRITE);
		err = iic_wait_ack();
		if (err == 0) {
			iic_sendbyte(info.command);
			err = iic_wait_ack();
			if (err == 0) {
				while (i < info.data_lens) {
					iic_sendbyte(writeData[i]);
					err = iic_wait_ack();
					i++;
				}
			}
		}
		iic_stop();
	return err;
}
