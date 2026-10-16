#ifndef INC_WF162A_H_
#define INC_WF162A_H_

#include "stm32g0xx_hal.h"
#include "CmdHelper.h"

/* Two supply rounds, each with current, Vout+, Vout- and Vout+ - Vout-. */
#define WF162A_SUPPLY_RESULT_LENGTH 16u
#define WF162A_RESULT_LENGTH 32u
#define WF162A_CURRENT_OFFSET 0u
#define WF162A_VOUT_PLUS_OFFSET 4u
#define WF162A_VOUT_MINUS_OFFSET 8u
#define WF162A_DIFF_OFFSET 12u

void AllTestFor162A(uint16_t *out, uint16_t out_lens);

#endif /* INC_WF162A_H_ */
