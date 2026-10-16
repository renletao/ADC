#include "wf162a.h"
#include "ADS1220.h"
#include "MCP4725.h"
#include "cmsis_os.h"
#include <string.h>

/*
 * WF162A-11Bar pinout from the supplied datasheet:
 *   1 = GND, 2 = Vout+, 3 = VDD, 4 = Vout-.
 *
 * The new board routes these three analogue nodes to the same ADS1220 mux
 * selections already used by WF100S: 0xA1 (AIN2-AVSS), 0xB1 (AIN3-AVSS)
 * and 0x51 (AIN2-AIN3). The mapping is kept in one place so it can be
 * changed with the board schematic without touching the result protocol.
 */
#define WF162A_CURRENT_CONFIG 0x01u
#define WF162A_VOUT_PLUS_CONFIG 0xA1u
#define WF162A_VOUT_MINUS_CONFIG 0xB1u
#define WF162A_DIFF_CONFIG 0x51u
#define WF162A_SETTLE_MS 150u
#define WF162A_SAMPLE_DELAY_MS 20u
#define WF162A_SAMPLE_ATTEMPTS 10u

static void wf162a_store_raw(uint16_t *dst, uint32_t raw)
{
    /* Product result buffers are 16-bit Modbus slots, but the existing
       protocol stores one raw ADC byte per slot (the same layout used by
       WF100S).  Keep the upper byte of each slot clear. */
    dst[0] = (uint8_t)raw;
    dst[1] = (uint8_t)(raw >> 8);
    dst[2] = (uint8_t)(raw >> 16);
    dst[3] = (uint8_t)(raw >> 24);
}

static uint32_t wf162a_sample(uint8_t config)
{
    uint32_t value = 0u;
    uint8_t attempt;

    ADS1220.config[0] = config;
    ADS1220_WriteRegister(0u, 1u, &ADS1220.config[0]);
    osDelay(WF162A_SAMPLE_DELAY_MS);
    /* Match the established product-test behavior: a disconnected ADC or a
       conversion timeout returns zero, so allow bounded retries before
       preserving the zero result in the output buffer. */
    for (attempt = 0u; attempt < WF162A_SAMPLE_ATTEMPTS; ++attempt) {
        value = GET_ADS1220_Val(&ADS1220);
        if (value != 0u) break;
    }
    return value;
}

static void wf162a_run_supply(uint16_t supply_mv, uint16_t *result)
{
    MCP4725_WriteData_Voltage(supply_mv);
    osDelay(WF162A_SETTLE_MS);
    wf162a_store_raw(result + WF162A_CURRENT_OFFSET,
                     wf162a_sample(WF162A_CURRENT_CONFIG));
    wf162a_store_raw(result + WF162A_VOUT_PLUS_OFFSET,
                     wf162a_sample(WF162A_VOUT_PLUS_CONFIG));
    wf162a_store_raw(result + WF162A_VOUT_MINUS_OFFSET,
                     wf162a_sample(WF162A_VOUT_MINUS_CONFIG));
    wf162a_store_raw(result + WF162A_DIFF_OFFSET,
                     wf162a_sample(WF162A_DIFF_CONFIG));
    MCP4725_WriteData_Voltage(0u);
}

void AllTestFor162A(uint16_t *out, uint16_t out_lens)
{
    uint8_t saved_config[4];

    if (!out || out_lens < WF162A_RESULT_LENGTH) return;
    memset(out, 0, out_lens * sizeof(*out));
    memcpy(saved_config, ADS1220.config, sizeof(saved_config));
    /* 0..15 = 3.3 V, 16..31 = 5.0 V. Each block preserves the historical
       four-byte little-endian ADS1220 result layout. */
    wf162a_run_supply(3300u, &out[0]);
    wf162a_run_supply(5000u, &out[WF162A_SUPPLY_RESULT_LENGTH]);
    memcpy(ADS1220.config, saved_config, sizeof(saved_config));
    ADS1220_WriteRegister(0u, 1u, &ADS1220.config[0]);
    MCP4725_WriteData_Voltage(0u);
}
