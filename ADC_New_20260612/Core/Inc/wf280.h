
#ifndef __WF280_H__
#define __WF280_H__

#include "stm32g0xx_hal.h"
#include "CmdHelper.h"

#define WF280_RESULT_LENGTH 2u

void WF280CheckOtpReg(uint16_t *out, uint16_t out_lens);

#endif //__WF280_H__
