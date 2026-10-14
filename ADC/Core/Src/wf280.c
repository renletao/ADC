
#include "wf280.h"
#include <string.h>
#include "cmsis_os.h"

uint16_t otp1[32];
uint16_t otp2[32];
uint16_t write_280reg[] = { 0, 0, 0x11, 0x70, 0x00, 0x01, 0x00}; //nc,nc,w,iicAddr,nc,wlens,wdata[0...wlens)
uint16_t read_280reg[]  = { 0, 0, 0x22, 0x70, 0x00, 0x03};//nc,nc,r,iicAddr,nc,rlens

void WF280CheckOtpReg(uint16_t *out, uint16_t out_lens) { 
  
  uint8_t i;
  uint16_t _out[3]={0};   
  uint8_t err = 0;
  memset(otp1, 0, 32* sizeof(uint16_t));
  memset(otp2, 0, 32* sizeof(uint16_t));
  
  for(i=0; i<32; i++) {
    write_280reg[6] = i;
    ParseI2CCmd(write_280reg, 0, _out, 0 );
     osDelay(2);
     ParseI2CCmd(read_280reg, 0, _out, 10 );
     osDelay(2);
     otp1[i] = _out[1];
     otp1[i] <<= 8;
     otp1[i] |= _out[2];
  }

  for(i=0; i<32; i++) {
     write_280reg[6] = 0x80+i;
     ParseI2CCmd(write_280reg, 0, _out, 0 );
     osDelay(2);
     ParseI2CCmd(read_280reg, 0, _out, 10 );
     osDelay(2);
     otp2[i] = _out[1];
     otp2[i] <<= 8;
     otp2[i] |= _out[2];
     //cmp
     if(otp2[i] != otp1[i]){
       err = i+1;
       break;
     }    
  }
  
  out[0] = err == 0 ? 0x01 : 0xFF;
  out[1] = err;
}

