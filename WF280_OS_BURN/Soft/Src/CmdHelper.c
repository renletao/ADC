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
#include "SoftIIC.h"
#include "flash.h"
#include "owi.h"
#include "tim.h"
#include "cmsis_os.h"
#include "WF280_OS.h"
#include "JH1400F.h"
#include "wf2200.h"


#define RETRY_COUNT 3

//一类：系统指令

#define TYPE1_GET_VERSION 		      0x104
#define TYPE1_OS_CHECK		 		      0x205
#define TYPE1_WF280_BURN			      0x206
#define TYPE1_WF280_M1				      0x207
#define TYPE1_WF280_OUTDATA		      0x208
#define TYPE1_WF280_COEFF			      0x209
#define TYPE1_WF280_2V_OUTDATA		  0x210
#define TYPE1_WF2200_3V3_COEFF	    0x401
#define TYPE1_WF2200_2V_COEFF		    0x402
#define TYPE1_WF2200_RAWDATA	      0x403
#define TYPE1_WF2200_SPI_DATA	      0x501
#define TYPE1_WF2200_6V2_OUTDATA	  0x504
#define TYPE1_WF2200_SET_OFFSET	    0x505
#define TYPE1_WF3050_FT_DATA	      0x567
#define TYPE1_WF5808_FT_DATA	      0x568

const uint8_t FIRMWARE_VERSION[] = { 0x01, 0x02 };

void OWI2VDD(uint8_t isOn){
  if(isOn) {
    HAL_GPIO_WritePin(OWI_GPIO_Port,OWI_Pin,GPIO_PIN_SET);
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};		  
    GPIO_InitStruct.Pin = OWI_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(OWI_GPIO_Port, &GPIO_InitStruct);       
  }
  else {
    //正常操作，应该是设置为GPIO_MODE_ANALOG
   // HAL_GPIO_WritePin(OWI_GPIO_Port,OWI_Pin,GPIO_PIN_RESET);
  }
}

void SetIoStatus(uint8_t osOn, uint8_t vddOn, uint8_t vppOn, uint8_t puOn,uint8_t m1On ) {
 
  HAL_GPIO_WritePin(OS_EN_GPIO_Port,OS_EN_Pin, osOn == 1 ? GPIO_PIN_RESET:GPIO_PIN_SET);
  HAL_GPIO_WritePin(VDD_EN_GPIO_Port,VDD_EN_Pin,vddOn == 1 ?GPIO_PIN_SET: GPIO_PIN_RESET);
  HAL_GPIO_WritePin(VPP_EN_GPIO_Port,VPP_EN_Pin,vppOn == 1 ?GPIO_PIN_SET: GPIO_PIN_RESET);
  HAL_GPIO_WritePin(PULL_UP_GPIO_Port,PULL_UP_Pin,puOn == 1 ?GPIO_PIN_SET: GPIO_PIN_RESET);
  HAL_GPIO_WritePin(M1_EN_GPIO_Port,M1_EN_Pin,m1On == 1 ?GPIO_PIN_SET: GPIO_PIN_RESET);
}
uint32_t ADC_Value[8];
void ParseSystemCmd(uint16_t *cmd, uint8_t cmd_lens, uint16_t *out, uint16_t out_lens) 
{
  uint8_t i;
  uint8_t j;
  uint8_t err;
  
	uint16_t cmdHead = cmd[1];
  uint8_t buf[32];
	switch (cmdHead) 
	{
		case TYPE1_GET_VERSION://104:获取版本号
			out[0] = FIRMWARE_VERSION[0];
			out[1] = FIRMWARE_VERSION[1];
		break;
				
		case TYPE1_OS_CHECK:
			
			ADC_MspInit();
      SetIoStatus(1,0,0,0,1);
      osDelay(50);
      Get_ADC_Value(ADC_Value);
      osDelay(10);
			out[0] = (uint8_t)ADC_Value[0];
			out[1] = (uint8_t)(ADC_Value[0] >> 8);
					
			out[2] = (uint8_t)ADC_Value[1];
			out[3] = (uint8_t)(ADC_Value[1] >> 8);
		
			out[4] = (uint8_t)ADC_Value[2];
			out[5] = (uint8_t)(ADC_Value[2] >> 8);
		
			out[6] = (uint8_t)ADC_Value[3];
			out[7] = (uint8_t)(ADC_Value[3] >> 8);
		
			out[8] = (uint8_t)ADC_Value[4];
			out[9] = (uint8_t)(ADC_Value[4] >> 8);
					
			out[10] = (uint8_t)ADC_Value[5];
			out[11] = (uint8_t)(ADC_Value[5] >> 8);
		
			out[12] = (uint8_t)ADC_Value[6];
			out[13] = (uint8_t)(ADC_Value[6] >> 8);
		
			out[14] = (uint8_t)ADC_Value[7];
			out[15] = (uint8_t)(ADC_Value[7] >> 8);
			SetIoStatus(0,0,0,0,0);
		break;
		
		case TYPE1_WF280_BURN:
			SetIoStatus(0,1,1,1,0);
      osDelay(100);
      do {
        IIC_Init(IIC_SDA_GPIO_Port,IIC_SDA_Pin,IIC_SCL_GPIO_Port,IIC_SCL_Pin);
        if ((err = WF280_SetCmdInNorMode(CMD_RESET, NULL, 0, 10, RETRY_COUNT)) != 0)
              break;
        osDelay(50);
        //CMD模式
				if ((err = WF280_SetCmdInCmdMode(CMD_START_CMD, NULL, 0, 3, RETRY_COUNT)) != 0)
						break;
        osDelay(10);
        for (i = 0; i < 3; i++) {//force write 3 times
          for (j = 0; j < 2; j++) {
            buf[0] = cmd[2+j] >> 8;
            buf[1] = cmd[2+j];
            if ((err = WF280_SetCmdInCmdMode(0x40 + j, buf, 2, 2, RETRY_COUNT)) != 0)
              break;
            osDelay(20);
          }
        }
      } while(0);      
      
      //无论如何，都要发一次NOR模式
			WF280_SetCmdInCmdMode(CMD_START_NOR, NULL, 0, 3, RETRY_COUNT);
      //
      out[0] = err;
			SetIoStatus(0,0,0,0,0);
		break;
				
    case TYPE1_WF280_COEFF:
      SetIoStatus(0,1,0,1,0);
      osDelay(100);
      do{
        IIC_Init(IIC_SDA_GPIO_Port,IIC_SDA_Pin,IIC_SCL_GPIO_Port,IIC_SCL_Pin);
        //复位
        if ((err = WF280_SetCmdInNorMode(CMD_RESET, NULL, 0, 10, RETRY_COUNT)) != 0)
          break;
        osDelay(10);
        for (j = 0; j < cmd[2]; j++) {
          if(j>=32) break;
          if ((err = WF280_GetChipData(j, buf, 3, RETRY_COUNT)) != 0)
            break;
          out[j+1] = ((uint16_t)buf[1] << 8) | buf[2];
          osDelay(5);
        }
      } while(0);
      //
      out[0] = err;
      SetIoStatus(0,0,0,0,0);
    break;
      
		case TYPE1_WF280_M1:
			SetIoStatus(0,1,0,1,1);
      osDelay(20);
      do{
        IIC_Init(IIC_SDA_GPIO_Port,IIC_SDA_Pin,IIC_SCL_GPIO_Port,IIC_SCL_Pin);
        i = 5;
        //复位
        if ((err = WF280_SetCmdInNorMode(CMD_RESET, NULL, 0, 10, RETRY_COUNT)) != 0)
          break;
        osDelay(50);
        for (j = 0; j < cmd[2]; j++) {
          if(j>=32) break;
          if ((err = WF280_GetChipData(j+0x80, buf, 3, RETRY_COUNT)) != 0)
            break;
          out[i++] = ((uint16_t)buf[1] << 8) | buf[2];
          osDelay(1);
        }
        //
        Get_ADC_Value(ADC_Value);
        osDelay(10);
        out[1] = (uint8_t)ADC_Value[0];
        out[2] = (uint8_t)(ADC_Value[0] >> 8);
        out[3] = (uint8_t)ADC_Value[3];
        out[4] = (uint8_t)(ADC_Value[3] >> 8);
      } while(0);
      //
      out[0] = err;
			SetIoStatus(0,0,0,0,0);
		break;
		
		case TYPE1_WF280_OUTDATA:
			SetIoStatus(0,1,0,1,0);
      osDelay(20);
      do{
        //复位
        IIC_Init(IIC_SDA_GPIO_Port,IIC_SDA_Pin,IIC_SCL_GPIO_Port,IIC_SCL_Pin);
        if ((err = WF280_SetCmdInNorMode(CMD_RESET, NULL, 0, 10, RETRY_COUNT)) != 0)
          break;
        osDelay(50);
        //发送0xAC采集指令
        buf[0] = cmd[2] >> 8;
        buf[1] = cmd[2];
        if ((err = WF280_SetCmdOnly(CMD_BT_MEAS, buf, 2, 5)) != 0)
          break;
        osDelay(cmd[3]);
        if ((err = WaitIdleInNor( 2, RETRY_COUNT)) != 0)
						break;

					if ((err = WF280_GetData(buf, 6)) != 0)
						break;
          
          for (j = 0; j < 6; j++)
            out[j+1] = buf[j];
          
      } while(0);
      out[0] = err;
    	SetIoStatus(0,0,0,0,0);
		break;
      
    case TYPE1_WF2200_3V3_COEFF://3v3读系数
      SetIoStatus(0,1,0,1,0);
      osDelay(20);
      do{
        IIC_Init(IIC_SDA_GPIO_Port,IIC_SDA_Pin,IIC_SCL_GPIO_Port,IIC_SCL_Pin);
        
        i = 2;//存档Pos
        //A4~A8        
        if ((err = WF2200_GetChipData(0xA4, buf, 5, RETRY_COUNT)) != 0)
          break;
        for (j = 0; j < 5; j++) {
          out[i++] = buf[j];
        }
        osDelay(1);
        //0xAE
        if ((err = WF2200_GetChipData(0xAE, buf, 0x0E, RETRY_COUNT)) != 0)
          break;
        for (j = 0; j < 0x0E; j++) {
          out[i++] = buf[j];
        }
        //OUT DATA
        if ((err = WF2200_GetData(buf, 0x05)) != 0)
          break;
        for (j = 0; j < 0x05; j++) {
          out[i++] = buf[j];
        }
        //A2
        if ((err = WF2200_GetChipData(0x02, buf, 1, RETRY_COUNT)) != 0)
          break;
        out[1] = buf[0];
      } while(0);      
      out[0] = err;
			SetIoStatus(0,0,0,0,0);
      break;
		
    case TYPE1_WF2200_2V_COEFF://2v读系数
      SetIoStatus(0,1,0,1,1);
      osDelay(20);      
      do{
        IIC_Init(IIC_SDA_GPIO_Port,IIC_SDA_Pin,IIC_SCL_GPIO_Port,IIC_SCL_Pin);
        
        i = 1;//存档Pos
        //A4~A8        
        if ((err = WF2200_GetChipData(0xA4, buf, 5, RETRY_COUNT)) != 0)
          break;
        for (j = 0; j < 5; j++) {
          out[i++] = buf[j];
        }
        osDelay(1);
        //0xAE
        if ((err = WF2200_GetChipData(0xAE, buf, 0x0E, RETRY_COUNT)) != 0)
          break;
        for (j = 0; j < 0x0E; j++) {
          out[i++] = buf[j];
        }
        //
        Get_ADC_Value(ADC_Value);
        osDelay(10);
        out[i++] = (uint8_t)ADC_Value[0];
        out[i++] = (uint8_t)(ADC_Value[0] >> 8);
        out[i++] = (uint8_t)ADC_Value[3];
        out[i] = (uint8_t)(ADC_Value[3] >> 8);
      } while(0);      
      out[0] = err;
			SetIoStatus(0,0,0,0,0);
      break;
      
    case TYPE1_WF2200_RAWDATA:
      SetIoStatus(0,1,0,1,0);
      osDelay(20);
      do{
        IIC_Init(IIC_SDA_GPIO_Port,IIC_SDA_Pin,IIC_SCL_GPIO_Port,IIC_SCL_Pin);
        i = 2;
        if ((err = WF2200_GetRawData(buf, 0x05)) != 0)
          break;
        for (j = 0; j < 0x05; j++) {
          out[i++] = buf[j];
        }
        //A2
        if ((err = WF2200_GetChipData(0x02, buf, 1, RETRY_COUNT)) != 0)
          break;
        out[1] = buf[0];
      } while(0);      
      out[0] = err;
			SetIoStatus(0,0,0,0,0);
      break;
      
     case TYPE1_WF280_2V_OUTDATA:       
			SetIoStatus(0,1,0,1,1);
      osDelay(200);
      do{
        //复位
        IIC_Init(IIC_SDA_GPIO_Port,IIC_SDA_Pin,IIC_SCL_GPIO_Port,IIC_SCL_Pin);
        if ((err = WF280_SetCmdInNorMode(CMD_RESET, NULL, 0, 10, RETRY_COUNT)) != 0)
          break;
        osDelay(50);
        //发送0xAC采集指令
        buf[0] = cmd[2] >> 8;
        buf[1] = cmd[2];
        if ((err = WF280_SetCmdOnly(CMD_BT_MEAS, buf, 2, 5)) != 0)
          break;
        osDelay(cmd[3]);
        if ((err = WaitIdleInNor( 2, RETRY_COUNT)) != 0)
						break;

					if ((err = WF280_GetData(buf, 6)) != 0)
						break;
          
          for (j = 0; j < 6; j++)
            out[j+1] = buf[j];
          
      } while(0);
      out[0] = err;
    	SetIoStatus(0,0,0,0,0);
      break;
      
     case TYPE1_WF2200_SPI_DATA:
      SetIoStatus(0,1,0,1,0);
      osDelay(20);
      WF2200_SPI_GetRawData(buf, 5);
      for (j = 0; j < 5; j++) out[j] = buf[j];
			SetIoStatus(0,0,0,0,0);
      break;
      
     case TYPE1_WF3050_FT_DATA:
        SetIoStatus(0,1,0,1,0);
        //owi输出供电
        //OWI2VDD(1); 
        osDelay(200);
        do{
          i=0;
          //ref sensor
          IIC_Init(USART_RX_GPIO_Port,USART_RX_Pin,USART_TX_GPIO_Port,USART_TX_Pin);
          if ((out[i++] = WF2200_GetData(buf, 0x05)) != 0) {
            break;
          }
          for (j = 0; j < 0x05; j++) {
            out[i++] = buf[j];
          }
          //ft sensor
          IIC_Init(IIC_SDA_GPIO_Port,IIC_SDA_Pin,IIC_SCL_GPIO_Port,IIC_SCL_Pin);
          if ((out[i++] = WF2200_GetData(buf, 0x05)) != 0) {
            break;
          }
          for (j = 0; j < 0x05; j++) {
            out[i++] = buf[j];
          }          
        } while(0);
        //OWI2VDD(0);        
        SetIoStatus(0,0,0,0,0);
       break;
        
     case TYPE1_WF2200_6V2_OUTDATA:
       //vpp供电的电阻R14要改为91kΩ，6.1v左右
        SetIoStatus(0,0,1,1,0);
         osDelay(200);
         do{
            i=1;
            IIC_Init(IIC_SDA_GPIO_Port,IIC_SDA_Pin,IIC_SCL_GPIO_Port,IIC_SCL_Pin);
            //A4~A8        
            if ((err = WF2200_GetChipData(0xA4, buf, 5, RETRY_COUNT)) != 0)
              break;
            for (j = 0; j < 5; j++) {
              out[i++] = buf[j];
            }
            //otp coef
            if ((err = WF2200_GetChipData(0xAE, buf, 0x0E, RETRY_COUNT)) != 0)
              break;
            for (j = 0; j < 0x0E; j++) {
              out[i++] = buf[j];
            }
            //
            if ((err = WF2200_GetData(buf, 0x05)) != 0) 
              break;            
            for (j = 0; j < 0x05; j++) {
              out[i++] = buf[j];
            } 
            
          } while(0);
          out[0] = err;
          SetIoStatus(0,0,0,0,0);
       break;
     
     case TYPE1_WF2200_SET_OFFSET:
       SetIoStatus(0,0,1,1,0);
       osDelay(200);
       do{
         i = 0; 
         IIC_Init(IIC_SDA_GPIO_Port,IIC_SDA_Pin,IIC_SCL_GPIO_Port,IIC_SCL_Pin);          
          if ((err = WF2200_SetOffset((uint8_t)cmd[2])) != 0)
            break;
        } while(0);      
        out[0] = err;
        SetIoStatus(0,0,0,0,0);
       break;
        
      case TYPE1_WF5808_FT_DATA:
        SetIoStatus(0,1,0,1,0);      
        osDelay(200);
        do{
          i=0;        
          //ft sensor
          IIC_Init(IIC_SDA_GPIO_Port,IIC_SDA_Pin,IIC_SCL_GPIO_Port,IIC_SCL_Pin);
          if ((out[i++] = WF5808_GetData(buf)) != 0) {
            break;
          }
          for (j = 0; j < 13; j++) {
            out[i++] = buf[j];
          }          
        } while(0);      
        SetIoStatus(0,0,0,0,0);
     break;
        
		default:
      break;
	}
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

	break;
	
	case 3:
		
	break;
	
	default:
			
	break;
	}
}



