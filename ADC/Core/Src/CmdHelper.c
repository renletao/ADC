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
#include "I2CHelper.h"
#include "flash.h"
#include "owi.h"
#include "tim.h"
#include "cmsis_os.h"
#include "wf280.h"
#include "wf2200.h"
#include "SoftUart.h"
#include "iwdg.h"
//一类：系统指令
//二类：串口通讯方式指令
//三类: IIC通讯方式指令
//四类: SPI通讯方式指令

WF183D_Dev WF183D;

const uint8_t FIRMWARE_VERSION[] = { 0x01, 0x09 };
#define DATA_LENTH					64
#define DATATMP_LENGTH  32
uint8_t dataTmp[DATATMP_LENGTH];
struct structI2CINFO i2cFromPC;

uint16_t _ch = 0;
int ADC_Value = 0;
extern uint16_t adc_buf[1];

/************tasks****************/
extern uint8_t IsTaskDone;
extern uint16_t sendBufBy[120];
extern uint8_t IsTask_mub;
/**********************************/

 
static void EXTILine4_15_Config(void)
 {
  GPIO_InitTypeDef   GPIO_InitStructure;
  __HAL_RCC_GPIOB_CLK_ENABLE();
  GPIO_InitStructure.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStructure.Pull = GPIO_NOPULL;
  GPIO_InitStructure.Pin = GPIO_PIN_7;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
  HAL_NVIC_SetPriority(EXTI4_15_IRQn, 2, 2);
  HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
 }
uint32_t tim;
void ParseSystemCmd(uint16_t *cmd, uint8_t cmd_lens, uint16_t *out, uint16_t out_lens) 
{
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
				ADC_Value = GET_ADS1220_val(&ADS1220);
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
			if(cmd[2]==0xFF)
			{
				HAL_GPIO_WritePin(PWR_GPIO_Port, PWR_Pin,GPIO_PIN_SET);
				HAL_GPIO_WritePin(SCL_GPIO_Port, SDA_Pin, GPIO_PIN_SET);
				HAL_GPIO_WritePin(SDA_GPIO_Port, SCL_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(OWI_GPIO_Port, OWI_Pin, GPIO_PIN_SET);
			}
			else
			{
				HAL_GPIO_WritePin(PWR_GPIO_Port, PWR_Pin,GPIO_PIN_RESET);
				HAL_GPIO_WritePin(SCL_GPIO_Port, SDA_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(SDA_GPIO_Port, SCL_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(OWI_GPIO_Port, OWI_Pin, GPIO_PIN_SET);
			}
			out[0] =HAL_GPIO_ReadPin(PWR_GPIO_Port, PWR_Pin);
		break;
			
		case TYPE1_GET_TASK_STATUS:
			out[0]= IsTaskDone;
			break;
		
		case TYPE1_GET_ALLTEST_DATA:
			memcpy( out, sendBufBy , sizeof(uint16_t)*out_lens);
			break;
        
    case TYPE1_WF3050_FT_DATA:
      WF3050FtData(out,out_lens);
      break;
		default:
		break;
	}
}


//void ParseSerialCmd(uint16_t *cmd, uint8_t cmd_lens, uint16_t *out, uint16_t out_lens) {
//  uint16_t sendLens = cmd[2];
//	uint16_t recvLens = cmd[4];
//	uint8_t SendBuff[DATA_LENTH] = {0};
//	uint8_t RecvBuff[DATA_LENTH] = {0};
//	for(uint8_t i = 0; i<sendLens;i++)
//	{
//		SendBuff[i] = cmd[i+6];
//	}
//	HAL_IWDG_Refresh(&hiwdg);
//	UART_Init(&uart1,9600,UART_RX_GPIO_Port,UART_RX_Pin,UART_TX_GPIO_Port,UART_TX_Pin);	
//	osDelay(10);
//	
//	Soft_Uart_Send(&uart1, SendBuff, sendLens);
//	Soft_Uart_Recv(&uart1, RecvBuff, recvLens);
//	
//	out[1] = (uint8_t)(1000000/uart1.Save_BuadRate);
//	out[0] = (uint8_t)((1000000/uart1.Save_BuadRate)>>8);
//	
//	for(uint8_t i = 0; i<recvLens;i++)
//	{
//		out[i+2] = RecvBuff[i];
//	}
//	
//	UART_DeInit(&uart1);	
//}
void ParseSerialCmd(uint16_t *cmd, uint8_t cmd_lens, uint16_t *out, uint16_t out_lens) {
	
	uint16_t sendLens = cmd[2];
	uint16_t sendTimeout = cmd[3];
	uint16_t recvLens = cmd[4];
	uint16_t recvTimeout = cmd[5];
	uint8_t BaudRatetestdata[4] = { 0x55, 0x04, 0x00, 0x75};
	memset(WF183D.Uart_RevBuff,0,8);
	memset(WF183D.Uart_SendBuff,0,8);
    WF183D.BaudRate = 0;
	WF183D.Uart_State = 1;
	for(uint8_t i = 0; i<sendLens;i++)
	{
		WF183D.Uart_SendBuff[i] = cmd[i+6];
	}
	HAL_UART_MspInit(&huart1);
	MX_USART1_UART_Init();
	EXTILine4_15_Config();
	HAL_UART_Transmit(&huart1,BaudRatetestdata,4,sendTimeout);
	osDelay(10);
	HAL_UART_MspInit(&huart1);
	MX_USART1_UART_Init();
	HAL_UART_Transmit(&huart1,WF183D.Uart_SendBuff,sendLens,sendTimeout);
	if (recvLens > 0) 
	{
		HAL_UART_Receive(&huart1,WF183D.Uart_RevBuff,recvLens,recvTimeout);
		out[1] = (uint8_t)(2000000/WF183D.BaudRate);
		out[0] = (uint8_t)((2000000/WF183D.BaudRate)>>8);
		for(uint8_t i = 0; i<recvLens;i++)
		{
			out[i+2] = WF183D.Uart_RevBuff[i];
		}
	}
	
	HAL_UART_MspDeInit(&huart1);
}

void ParseI2CCmd(uint16_t *cmd, uint8_t cmd_lens, uint16_t *out, uint16_t out_lens) { 
  uint8_t i;
	//format: type_channel_r/w_busAddr_command_dataLens_datas[N]
	i2cFromPC.channel = cmd[1];
	i2cFromPC.bus_addr = cmd[3];
	i2cFromPC.start_reg = cmd[4];
	i2cFromPC.data_lens = cmd[5];
	if (cmd[2] == 0x11) 
  { //iic_write
		if (i2cFromPC.data_lens < DATATMP_LENGTH) 
    {
			memset(dataTmp, 0, sizeof(dataTmp));
			for (i = 0; i < i2cFromPC.data_lens; i++) 
      {
				dataTmp[i] = cmd[6 + i];
			}	
			out[0] = I2C_WriteNByte(i2cFromPC, dataTmp);
		} 
    else
    {
			out[0] = 0xF2;
    }
	}
  else if (cmd[2] == 0x22)
  { //iic_read
		if (i2cFromPC.data_lens < out_lens) 
    {
			out[i2cFromPC.data_lens] = I2C_ReadNByte(i2cFromPC, out);		
		}
    else
    {
			out[0] = 0xF2;
    }
	}
  else
		out[0] = 0xFE;
	I2C_IDLE();
}

void ParseProductCmd(uint16_t *cmd, uint8_t cmd_lens, uint16_t *out, uint16_t out_lens)
{
	OWI_Device.cycle = 0;
	WF183D.Owi_Buff[0] = 0;
	WF183D.Owi_Buff[1] = 0;
	WF183D.Owi_Buff[2] = 0;
	WF183D.Owi_State = 0;
	WF183D.Owi_State = OWI(WF183D.Owi_Buff,200);
	osDelay(100);
	WF183D.Owi_State = OWI(WF183D.Owi_Buff,200);
	WF183D.Owi_BaudRate = OWI_Device.cycle;
	out[0] = WF183D.Owi_BaudRate;
	out[1] = WF183D.Owi_Buff[0];
	out[2] = WF183D.Owi_Buff[1];
	out[3] = WF183D.Owi_Buff[2];
	
}
/*
 *
 */
void Parse(uint16_t *cmd, uint8_t cmd_lens, uint16_t *out, uint16_t out_lens) {
	uint16_t cmdType = cmd[0];
	switch (cmdType) {
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
		ParseProductCmd(cmd, cmd_lens, out, out_lens);
		break;
	case 6:
		iic_init(GPIOB , GPIO_PIN_6, GPIOB, GPIO_PIN_7);
	case 7:
		iic_init(SDA_GPIO_Port, SDA_Pin, SCL_GPIO_Port, SCL_Pin);
		break;
	case 8:
		IsTask_mub = WF183DE;
		IsTaskDone = 1;        
	break;
	case 9:
		IsTask_mub = WF100S;
		IsTaskDone = 1;        
	break;
	case 10:
		IsTask_mub = WF2200;
		IsTaskDone = 1;        
	break;
  case 11:
    WF280CheckOtpReg(out,out_lens);
    break;
	}
}

void HAL_GPIO_EXTI_Rising_Callback(uint16_t GPIO_Pin)
{	
	if(WF183D.Uart_State)
	{
		WF183D.BaudRate = GET_TIM_COUNTER;
		WF183D.Uart_State = 0;
	}
}
void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin)
{
	if(WF183D.Uart_State)
	{
		RESET_TIM_COUNTER();
	}
}

// 拷贝 src 到 dest，然后清零 src
void CopyAndClear(uint16_t* src, uint16_t* dest, size_t len) {
    if (!src || !dest || len == 0) return;
    size_t bytes = len * sizeof(uint16_t);
    memcpy(dest, src, bytes);
    memset(src, 0, bytes);
}
   







