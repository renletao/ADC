
#include "wf280.h"
#include "I2CHelper.h"
#include <string.h>
#include "cmsis_os.h"

uint16_t otp1[32];
uint16_t otp2[32];
static uint16_t write_280reg[] = { 3, 0, 2, 0, 0, 0, 0x70, 0x00 }; // Latest format: type, reserved, send_len, reserved, recv_len, reserved, address, payload
static const uint16_t read_280reg[] = { 3, 0, 1, 0, 3, 0, 0x70 };// Latest format: type, reserved, send_len, reserved, recv_len, reserved, address, payload

void WF280CheckOtpReg(uint16_t *out, uint16_t out_lens) {
  if (out == NULL || out_lens < WF280_RESULT_LENGTH) return;
  memset(out, 0, out_lens * sizeof(*out));

  uint8_t i;
  uint16_t _out[4]={0};
  uint8_t err = 0;
  memset(otp1, 0, 32* sizeof(uint16_t));
  memset(otp2, 0, 32* sizeof(uint16_t));

  for(i=0; i<32; i++) {
    write_280reg[7] = i;
    if (ProductI2CTransfer(write_280reg, sizeof(write_280reg) / sizeof(write_280reg[0]), _out, sizeof(_out) / sizeof(_out[0])) != 0) { out[0] = 0xFF; out[1] = i + 1u; return; }
     osDelay(2);
     if (ProductI2CTransfer(read_280reg, sizeof(read_280reg) / sizeof(read_280reg[0]), _out, sizeof(_out) / sizeof(_out[0])) != 0) { out[0] = 0xFF; out[1] = i + 1u; return; }
     osDelay(2);
     otp1[i] = _out[1];
     otp1[i] <<= 8;
     otp1[i] |= _out[2];
  }

  for(i=0; i<32; i++) {
     write_280reg[7] = 0x80+i;
     if (ProductI2CTransfer(write_280reg, sizeof(write_280reg) / sizeof(write_280reg[0]), _out, sizeof(_out) / sizeof(_out[0])) != 0) { out[0] = 0xFF; out[1] = i + 1u; return; }
     osDelay(2);
     if (ProductI2CTransfer(read_280reg, sizeof(read_280reg) / sizeof(read_280reg[0]), _out, sizeof(_out) / sizeof(_out[0])) != 0) { out[0] = 0xFF; out[1] = i + 1u; return; }
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
