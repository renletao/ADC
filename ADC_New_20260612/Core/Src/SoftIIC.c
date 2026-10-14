/*
 * SoftIIC.c
 *
 *  Created on: Aug 3, 2020
 *      Author: ITry
 */
#include "SoftIIC.h"
#include <string.h>
I2CInfo i2c1;
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
void IIC_Init(I2CInfo *info,GPIO_TypeDef *SDA_PORT, uint16_t SDA_PIN, GPIO_TypeDef *SCL_PORT, uint16_t SCL_PIN)
{
	info->portSda = SDA_PORT;
	info->portScl = SCL_PORT;
	info->pinSda = SDA_PIN;
	info->pinScl = SCL_PIN;
	IIC_MspInit(info->portSda,info->pinSda,info->portScl,info->pinScl);
}

/**
 *iic取消初始化
 */
void IIC_DeInit(I2CInfo *info)
{
	IIC_DeMspInit(info->portSda,info->pinSda,info->portScl,info->pinScl);
	info->portSda = NULL;
	info->portScl = NULL;
	info->pinSda = 0;
	info->pinScl = 0;
}

/**
 *iic启动:当SCL处于高电平状态时，SDA出现一个下降沿,即产生IIC启动信号
 */
void iic_start(I2CInfo *info)
{
	HAL_GPIO_WritePin(info->portSda, info->pinSda, GPIO_PIN_SET);
	HAL_GPIO_WritePin(info->portScl, info->pinScl, GPIO_PIN_SET);
	delay_us(20);
	HAL_GPIO_WritePin(info->portSda, info->pinSda, GPIO_PIN_RESET);
	delay_us(20);
	HAL_GPIO_WritePin(info->portScl, info->pinScl, GPIO_PIN_RESET);
}

/**
 *iic停止:当SCL处于高电平状态时，SDA出现一个上升沿,即产生IIC停止信号
 */
void iic_stop(I2CInfo *info) 
{
	HAL_GPIO_WritePin(info->portSda, info->pinSda, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(info->portScl, info->pinScl, GPIO_PIN_SET);
	delay_us(20);
	HAL_GPIO_WritePin(info->portSda, info->pinSda, GPIO_PIN_SET);
	delay_us(20);
}

/**
 *IIC等待应答信号
 *ack_status： 应答状态，0表示应答，1表示设备无响应
 */
uint8_t iic_wait_ack(I2CInfo *info) 
{
	uint8_t err_time = 0;
	HAL_GPIO_WritePin(info->portSda, info->pinSda, GPIO_PIN_SET);
	delay_us(20);
	HAL_GPIO_WritePin(info->portScl, info->pinScl, GPIO_PIN_SET);
	delay_us(20);
	while ((HAL_GPIO_ReadPin(info->portSda, info->pinSda) == GPIO_PIN_SET))
	{
		err_time++;
		delay_us(1);
		if (err_time > 250)
		{
			iic_stop(info);
			return 1;
		}
	}
	HAL_GPIO_WritePin(info->portScl, info->pinScl, GPIO_PIN_RESET);
	delay_us(20);
	return 0;
}

/**
 *主机（主控制器）产生应答信号
 */
void iic_ack(I2CInfo *info)
{
	HAL_GPIO_WritePin(info->portSda, info->pinSda, GPIO_PIN_RESET);
	delay_us(20);
	HAL_GPIO_WritePin(info->portScl, info->pinScl, GPIO_PIN_SET);
	delay_us(20);
	HAL_GPIO_WritePin(info->portScl, info->pinScl, GPIO_PIN_RESET);
	delay_us(20);
	HAL_GPIO_WritePin(info->portSda, info->pinSda, GPIO_PIN_SET);
}

/**
 * 主机（主控制器）产生不应答信号
 */
void iic_nack(I2CInfo *info) 
{
	HAL_GPIO_WritePin(info->portSda, info->pinSda, GPIO_PIN_SET);
	delay_us(20);
	HAL_GPIO_WritePin(info->portScl, info->pinScl, GPIO_PIN_SET);
	delay_us(20);
	HAL_GPIO_WritePin(info->portScl, info->pinScl, GPIO_PIN_RESET);
	delay_us(20);
}

/**
 *IIC发送1byte数据
 */
void iic_sendbyte(I2CInfo *info,uint8_t byte) 
{
	uint8_t i;
	/** 发送一个字节的高7位 */
	for (i = 0; i < 8; i++) 
	{
		if (byte & 0x80) 
		{
			HAL_GPIO_WritePin(info->portSda, info->pinSda, GPIO_PIN_SET);
		}
		else
		{
			HAL_GPIO_WritePin(info->portSda, info->pinSda, GPIO_PIN_RESET);
		}
		byte <<= 1;
		delay_us(10);
		HAL_GPIO_WritePin(info->portScl, info->pinScl, GPIO_PIN_SET);
		delay_us(20);
		HAL_GPIO_WritePin(info->portScl, info->pinScl, GPIO_PIN_RESET);
		delay_us(10);
	}
}

/**
 *IIC读取1byte数据
 */
uint8_t iic_readbyte(I2CInfo *info,uint8_t ack)
{
	uint8_t i;
	uint8_t recv_value = 0;
	HAL_GPIO_WritePin(info->portSda, info->pinSda, GPIO_PIN_SET);
  delay_us(20);
	for (i = 0; i < 8; i++) 
	{
		HAL_GPIO_WritePin(info->portScl, info->pinScl, GPIO_PIN_SET);
		delay_us(10);		
    recv_value <<= 1;
		if((HAL_GPIO_ReadPin(info->portSda, info->pinSda) == GPIO_PIN_SET)) 
		{
			recv_value++;
		}
    delay_us(10);		
		HAL_GPIO_WritePin(info->portScl, info->pinScl, GPIO_PIN_RESET);
		delay_us(20);
	}
	if(!ack)
	{
		iic_nack(info);
	}
	else
	{
		iic_ack(info);
	}
	return recv_value;
}
/**
 *iic多字节读取
 */
uint8_t I2C_ReadNByte(I2CInfo *info,uint8_t addr,uint8_t *readData,uint32_t lenth)
{
	uint8_t i = 0;
	uint8_t err = 0;
	if(lenth > 0) memset(readData, 0, (lenth * sizeof(uint8_t)));
	else return err;
	iic_start(info);
	iic_sendbyte(info,addr | IIC_READ);
	err = iic_wait_ack(info);
	if (err) { iic_stop(info); return err; }
	for (i = 0; i < lenth; i++) 
	{
		if (i != (lenth - 1)) 
		{
			readData[i] = iic_readbyte(info,1); 
		} 
		else
		{
			readData[i] = iic_readbyte(info,0); 
		}
	}
	iic_stop(info);
	return err;
}
/**
 *iic多字节发送
 */
uint8_t I2C_WriteNByte(I2CInfo *info, const uint8_t *writeData,uint32_t lenth) 
{
	uint8_t err = 0;
	if(lenth <= 0)return err;
	iic_start(info);
	for (uint8_t i = 0; i < lenth; i++) 
	{
		iic_sendbyte(info,writeData[i]);
		err = iic_wait_ack(info);
		if(err) break;
	}
	iic_stop(info);
	return err;
}

void I2C_IDLE(I2CInfo *info)
{
	IIC_DeInit(info);
}
