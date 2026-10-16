#include "delay.h"
#include "tim.h"

void delay_us(uint32_t us)
{
  uint16_t delay=0xffff-(us-2) -5;//-5				
  __HAL_TIM_SET_COUNTER(&htim1,delay);		
  while(delay<0xffff-5)
  {
     delay=__HAL_TIM_GET_COUNTER(&htim1); 
  }
}


