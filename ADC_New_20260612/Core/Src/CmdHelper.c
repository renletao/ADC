/*
 * CmdHelper.c
 *
 *  Created on: Feb 17, 2022
 *      Author: ITry
 */

#include <string.h>
#include "main.h"
#include "CmdHelper.h"
#include "usart.h"
#include "ads1220.h"
#include "flash.h"
#include "owi.h"
#include "tim.h"
#include "cmsis_os.h"
#include "SoftIIC.h"
#include "MCP4725.h"
#include "SoftUart.h"
//一类：系统指令
//二类：串口通讯方式指令
//三类: IIC通讯方式指令
//四类: SPI通讯方式指令

const uint8_t FIRMWARE_VERSION[] = { 0x01, 0x00 };
#define DATA_LENTH					64

/************tasks****************/

/**********************************/

void ParseSystemCmd(uint16_t *cmd, uint8_t cmd_lens, uint16_t *out, uint16_t out_lens) 
{
	uint16_t Vout = 0;
	uint32_t ADC_Value = 0;
	uint16_t cmdHead = cmd[1];
	switch (cmdHead) 
	{
		
		case TYPE1_GET_VERSION://104:获取版本号
			out[0] = FIRMWARE_VERSION[0];
			out[1] = FIRMWARE_VERSION[1];
		break;
		
		case TYPE1_GET_ADC://105：获取adc值
			for(uint8_t i = 0;i<10;i++)
			{
				ADC_Value = GET_ADS1220_Val(&ADS1220);
				out[0] = (uint8_t)(ADC_Value); 
				out[1] = (uint8_t)(ADC_Value>>8);
				out[2] = (uint8_t)(ADC_Value>>16);
				out[3] = (uint8_t)(ADC_Value>>24);
				if(ADC_Value)break;
			}
			break;
		
		case TYPE1_SET_ADC://107：设置adc模式不保存参数
			ADS1220.config[0] = cmd[2];
			ADS1220.config[1] = cmd[3];
			ADS1220.config[2] = cmd[4];
			ADS1220.config[3] = cmd[5];
			ADS1220_Init(&ADS1220);
			out[0] = ADS1220.state;
		break;
		
		case TYPE1_SAVE_ADC://200：设置adc模式，保存参数至flash
			ADS1220.config[0] = cmd[2];
			ADS1220.config[1] = cmd[3];
			ADS1220.config[2] = cmd[4];
			ADS1220.config[3] = cmd[5];
			ADS1220_Init(&ADS1220);
			Write_Flash(FLASH_USER_START_ADDR,(uint64_t*)&ADS1220, 16);
        
			out[0] = ADS1220.state;
		break;
		
		case TYPE1_READ_ADC://108：读adc参数
			out[0] = ADS1220.config[0];
			out[1] = ADS1220.config[1];
			out[2] = ADS1220.config[2];
			out[3] = ADS1220.config[3];
		break;
		
		case TYPE1_SET_VDD://201：设置adc供电使能 
		  Vout = ((uint16_t)(cmd[3] << 8) | cmd[2]);
			MCP4725_WriteData_Voltage(Vout);
		break;
				
		case TYPE1_SET_VPP://202：设置VPP供电使能 
			HAL_GPIO_WritePin(VPP_EN_GPIO_Port, VPP_EN_Pin, cmd[2] ? GPIO_PIN_SET:GPIO_PIN_RESET);
			out[0] =HAL_GPIO_ReadPin(VPP_EN_GPIO_Port, VPP_EN_Pin);
		break;
			    
		default:
		break;
	}
}

void ParseSerialCmd(uint16_t *cmd, uint8_t cmd_lens, uint16_t *out, uint16_t out_lens)
{
	uint16_t sendLens = cmd[2];
	uint16_t sendTimeout = cmd[3];
	uint16_t recvLens = cmd[4];
	uint16_t recvTimeout = cmd[5];
	
	uint8_t SendBuff[DATA_LENTH] = {0};
	uint8_t RecvBuff[DATA_LENTH] = {0};
	for(uint8_t i = 0; i<sendLens;i++)
	{
		SendBuff[i] = cmd[i+6];
	}
	
	UART_Init(&uart1,9600,UART_RX_GPIO_Port,UART_RX_Pin,UART_TX_GPIO_Port,UART_TX_Pin);	
	osDelay(10);
	
	Soft_Uart_Send(&uart1, SendBuff, sendLens,sendTimeout);
	Soft_Uart_Recv(&uart1, RecvBuff, recvLens,recvTimeout);
	
	out[1] = (uint8_t)(1000000/uart1.Save_BuadRate);
	out[0] = (uint8_t)((uint16_t)(1000000/uart1.Save_BuadRate)>>8);
	
	for(uint8_t i = 0; i<recvLens;i++)
	{
		out[i+2] = RecvBuff[i];
	}
	
	UART_DeInit(&uart1);	
}

void ParseI2CCmd(uint16_t *cmd, uint8_t cmd_lens, uint16_t *out, uint16_t out_lens) 
{ 
	uint16_t sendLens = cmd[2];
	uint16_t recvLens = cmd[4];
	uint8_t SendBuff[DATA_LENTH] = {0};
	uint8_t RecvBuff[DATA_LENTH] = {0};
	
	for(uint8_t i = 0; i<sendLens;i++)
	{
		SendBuff[i] = cmd[i+6];
	}
	
	IIC_Init(&i2c1,I2C_SDA_GPIO_Port,I2C_SDA_Pin,I2C_SCL_GPIO_Port,I2C_SCL_Pin);
	osDelay(10);
	
	I2C_WriteNByte(&i2c1,SendBuff,sendLens);
	I2C_ReadNByte(&i2c1,SendBuff[0],RecvBuff,recvLens);
		
	for(uint8_t i = 0; i<recvLens;i++)
	{
		out[i+2] = RecvBuff[i];
	}
	
	IIC_DeInit(&i2c1);
}

void ParseOwiCmd(uint16_t *cmd, uint8_t cmd_lens, uint16_t *out, uint16_t out_lens)
{
	uint8_t RecvBuff[DATA_LENTH] = {0};
	OWI_Device.cycle = 0;
	OWI(RecvBuff,200);
	osDelay(100);
	OWI(RecvBuff,200);
	out[0] = OWI_Device.cycle;
	out[1] = RecvBuff[0];
	out[2] = RecvBuff[1];
	out[3] = RecvBuff[2];
}
/*
 *
 */
void Parse(uint16_t *cmd, uint8_t cmd_lens, uint16_t *out, uint16_t out_lens) 
{
	uint16_t cmdType = cmd[0];
	switch (cmdType) 
	{
		
		case 1:
			ParseSystemCmd(cmd, cmd_lens, out, out_lens);
		break;
		
		case 2:
			ParseSerialCmd(cmd, cmd_lens, out, out_lens);
		break;
		
		case 3:
			ParseI2CCmd(cmd, cmd_lens, out, out_lens);
		break;
		
		case 5:
			ParseOwiCmd(cmd, cmd_lens, out, out_lens);
		break;
		
		default:
		break;
	}
}

// 拷贝 src 到 dest，然后清零 src
void CopyAndClear(uint16_t* src, uint16_t* dest, size_t len) 	
{
	if (!src || !dest || len == 0) return;
	size_t bytes = len * sizeof(uint16_t);
	memcpy(dest, src, bytes);
	memset(src, 0, bytes);
}
   







