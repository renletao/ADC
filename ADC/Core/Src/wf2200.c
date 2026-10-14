
#include "wf2200.h"
#include "SoftIIC.h"
#include "cmsis_os.h"
#include "I2CHelper.h"
#include "SoftIIC_PP.h"
#include "iwdg.h"

/************WF2200****************/
  uint16_t iic_write_reset[] = { 0, 0, 0x11, 0xDA, 0x00, 0x02, 0x00, 0x24}; //nc,nc,w,iicAddr,nc,wlens,wdata[0...wlens)
	uint16_t iic_write_reg_start[] = { 0, 0, 0x11, 0xDA, 0x00, 0x01, 0xA4}; //nc,nc,w,iicAddr,nc,wlens,wdata[0...wlens)
	uint16_t iic_read_reg[] = { 0, 0, 0x22, 0xDA, 0x00, 0x04};//nc,nc,r,iicAddr,nc,rlens
	uint16_t iic_write_otp_start[] = { 0, 0, 0x11, 0xDA, 0x00, 0x01, 0xAE};
	uint16_t iic_read_otp[] = { 0, 0, 0x22, 0xDA, 0x00, 0x0E};
	uint16_t iic_write_data_cmd1[] = { 0, 0, 0x11, 0xDA, 0x00, 0x02, 0x30, 0x0A};
  uint16_t iic_write_data_cmd2[] = { 0, 0, 0x11, 0xDA, 0x00, 0x02, 0x30, 0x0B};
	uint16_t iic_write_data_start[] = { 0, 0, 0x11, 0xDA, 0x00, 0x01, 0x06};
	uint16_t iic_read_data[] = { 0, 0, 0x22, 0xDA, 0x00, 0x05};  
/**********************************/
  
 uint16_t sys_adc_2200[] = { 0, TYPE1_GET_ADC };
 uint16_t sys_vdd_off_2200[] = { 0, TYPE1_SET_VDD,0x00 };
 uint16_t sys_vdd_on_2200[] = { 0, TYPE1_SET_VDD,0xFF };
  
void AllTestFor2200(uint16_t *out, uint16_t out_lens){ 
	uint16_t _out[16]={0}; 
  uint8_t i;  
	//oppen power
  ParseSystemCmd(sys_vdd_on_2200, 2, _out, 4);
  iic_init(SDA_GPIO_Port, SDA_Pin, SCL_GPIO_Port, SCL_Pin);
  osDelay(50);
  ParseI2CCmd(iic_write_reset, 0, _out, 0 );
  osDelay(100);
  //0~3 | 4~26 | 27~49 | 50~72 | 73~76
  //sleep current  ParseSystemCmd   
  ParseSystemCmd(sys_adc_2200, 1, _out, 4);      
  CopyAndClear(_out, &out[0], 4);
  for(i=0; i<3; i++) {
    //id          ParseI2CCmd
    ParseI2CCmd(iic_write_reg_start, 0, _out, 0 );
    ParseI2CCmd(iic_read_reg, 0, _out, 10 );
    CopyAndClear(_out, &out[4+23*i], 4);
        
    //otp data    ParseI2CCmd
    ParseI2CCmd(iic_write_otp_start, 0, _out, 0 );            
    ParseI2CCmd(iic_read_otp, 0, _out, 16 );
    CopyAndClear(_out, &out[8+23*i], 14);
       
    //out data    ParseI2CCmd
    if(i==2) ParseI2CCmd(iic_write_data_cmd2, 0, _out, 0 );  
    else ParseI2CCmd(iic_write_data_cmd1, 0, _out, 0 );  
    osDelay(5);
    
    //read temp and press
    ParseI2CCmd(iic_write_data_start, 0, _out, 0 );
    ParseI2CCmd(iic_read_data, 0, _out, 10 );  
    CopyAndClear(_out, &out[22+23*i], 5);	
  }
  //work current     ParseSystemCmd
  ParseSystemCmd(sys_adc_2200, 1, _out, 4);
  CopyAndClear(_out, &out[73], 4);   
	
	//Close power
	ParseSystemCmd(sys_vdd_off_2200, 2, _out, 4);

}


void WF3050FtData(uint16_t *out, uint16_t out_lens) {
  uint16_t _out[16]={0}; 
  uint8_t i;  
  uint8_t j; 
  uint8_t buf[32];
	//oppen power
  ParseSystemCmd(sys_vdd_on_2200, 2, _out, 4);  
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  osDelay(150);
  HAL_IWDG_Refresh(&hiwdg);
  do{
      i=0;
      //ref sensor
      pp_iic_init(GPIOB, GPIO_PIN_7, GPIOB, GPIO_PIN_6);
      if ((out[i++] = PP_WF2200_GetData(buf, 0x05)) != 0) {
       // break;
      }
      for (j = 0; j < 0x05; j++) {
        out[i++] = buf[j];
      }
      HAL_IWDG_Refresh(&hiwdg);
      //ft sensor
      iic_init(SDA_GPIO_Port, SDA_Pin, SCL_GPIO_Port, SCL_Pin);
      if ((out[i++] = WF2200_GetData(buf, 0x05)) != 0) {
       // break;
      }
      for (j = 0; j < 0x05; j++) {
        out[i++] = buf[j];
      }          
    } while(0);
  
    //Close power
	ParseSystemCmd(sys_vdd_off_2200, 2, _out, 4);
}


