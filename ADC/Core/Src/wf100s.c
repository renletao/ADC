
#include "wf100s.h"
#include "cmsis_os.h"
#include "ads1220.h"


int ADC_Value_100s = 0;

void AllTestFor100S(uint16_t *out, uint16_t out_lens){ 
	HAL_GPIO_WritePin(PWR_GPIO_Port, PWR_Pin,GPIO_PIN_SET);
	do 
	{
		//工作电流
		ADS1220.config[0] = 0x01;
		ADS1220_WriteRegister(0x00, 1, &ADS1220.config[0]);
		osDelay(20);
		for(uint8_t i = 0;i<10;i++)
		{
			ADC_Value_100s = GET_ADS1220_val(&ADS1220);
			out[0] = (uint8_t)(ADC_Value_100s); 
			out[1] = (uint8_t)(ADC_Value_100s>>8);
			out[2] = (uint8_t)(ADC_Value_100s>>16);
			out[3] = (uint8_t)(ADC_Value_100s>>24);
			if(ADC_Value_100s)break;
		}
		
		//半桥电压
		ADS1220.config[0] = 0xA1;
		ADS1220_WriteRegister(0x00, 1, &ADS1220.config[0]);
		osDelay(20);
		for(uint8_t i = 0;i<10;i++)
		{
			ADC_Value_100s = GET_ADS1220_val(&ADS1220);
			out[4] = (uint8_t)(ADC_Value_100s); 
			out[5] = (uint8_t)(ADC_Value_100s>>8);
			out[6] = (uint8_t)(ADC_Value_100s>>16);
			out[7] = (uint8_t)(ADC_Value_100s>>24);
			if(ADC_Value_100s)break;
		}
		
		//工作电流
		ADS1220.config[0] = 0x01;
		ADS1220_WriteRegister(0x00, 1, &ADS1220.config[0]);
		osDelay(20);
		for(uint8_t i = 0;i<10;i++)
		{
			ADC_Value_100s = GET_ADS1220_val(&ADS1220);
			if(ADC_Value_100s)break;
		}
		
		//半桥电压
		ADS1220.config[0] = 0xB1;
		ADS1220_WriteRegister(0x00, 1, &ADS1220.config[0]);
		osDelay(20);
		for(uint8_t i = 0;i<10;i++)
		{
			ADC_Value_100s = GET_ADS1220_val(&ADS1220);
			out[8] = (uint8_t)(ADC_Value_100s); 
			out[9] = (uint8_t)(ADC_Value_100s>>8);
			out[10] = (uint8_t)(ADC_Value_100s>>16);
			out[11] = (uint8_t)(ADC_Value_100s>>24);
			if(ADC_Value_100s)break;
		}
		//工作电流
		ADS1220.config[0] = 0x01;
		ADS1220_WriteRegister(0x00, 1, &ADS1220.config[0]);
		osDelay(20);
		for(uint8_t i = 0;i<10;i++)
		{
			ADC_Value_100s = GET_ADS1220_val(&ADS1220);
			if(ADC_Value_100s)break;
		}
		
		//差分电压
		ADS1220.config[0] = 0x51;
		ADS1220_WriteRegister(0x00, 1, &ADS1220.config[0]);
		osDelay(20);
		for(uint8_t i = 0;i<10;i++)
		{
			ADC_Value_100s = GET_ADS1220_val(&ADS1220);
			out[12] = (uint8_t)(ADC_Value_100s); 
			out[13] = (uint8_t)(ADC_Value_100s>>8);
			out[14] = (uint8_t)(ADC_Value_100s>>16);
			out[15] = (uint8_t)(ADC_Value_100s>>24);
			if(ADC_Value_100s)break;
		}
	}
	while(0);    
	HAL_GPIO_WritePin(PWR_GPIO_Port, PWR_Pin,GPIO_PIN_RESET);
}

