#!/usr/bin/env python3
"""Verify that GET_ADS1220_Val discards one conversion and returns the next."""
from pathlib import Path
import os
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]

HAL = r'''
#ifndef TEST_HAL_H
#define TEST_HAL_H
#include <stdint.h>
typedef struct { uint32_t dummy; } GPIO_TypeDef;
typedef struct { uint32_t dummy; } SPI_HandleTypeDef;
typedef enum { GPIO_PIN_RESET, GPIO_PIN_SET } GPIO_PinState;
#define GPIOB ((GPIO_TypeDef *)1)
#define GPIO_PIN_0 1u
void HAL_SPI_Transmit(SPI_HandleTypeDef *, uint8_t *, uint16_t, uint32_t);
void HAL_SPI_Receive(SPI_HandleTypeDef *, uint8_t *, uint16_t, uint32_t);
GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef *, uint16_t);
#endif
'''

MAIN = r'''
#ifndef TEST_MAIN_H
#define TEST_MAIN_H
#include "stm32g0xx_hal.h"
void delay_us(uint32_t);
#endif
'''

SPI = r'''
#ifndef TEST_SPI_H
#define TEST_SPI_H
#include "stm32g0xx_hal.h"
extern SPI_HandleTypeDef hspi1;
#endif
'''

TIM = "#ifndef TEST_TIM_H\n#define TEST_TIM_H\n#endif\n"
CMSIS = "#ifndef TEST_CMSIS_H\n#define TEST_CMSIS_H\n#include <stdint.h>\nvoid osDelay(uint32_t);\n#endif\n"

HARNESS = r'''
#include <assert.h>
#include <stdio.h>
#include "ADS1220.h"

SPI_HandleTypeDef hspi1;
static unsigned starts;
static unsigned rx_index;

void HAL_SPI_Transmit(SPI_HandleTypeDef *h, uint8_t *data, uint16_t n, uint32_t timeout)
{
    (void)h; (void)n; (void)timeout;
    if (data[0] == ADS1220_START) ++starts;
    if (data[0] == ADS1220_RDATA) rx_index = 0;
}

void HAL_SPI_Receive(SPI_HandleTypeDef *h, uint8_t *data, uint16_t n, uint32_t timeout)
{
    static const uint8_t stale[] = {0x01, 0x02, 0x03};
    static const uint8_t fresh[] = {0x80, 0x00, 0x01};
    (void)h; (void)n; (void)timeout;
    data[0] = starts == 1 ? stale[rx_index] : fresh[rx_index];
    ++rx_index;
}

GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef *p, uint16_t pin)
{
    (void)p; (void)pin; return GPIO_PIN_SET;
}

void delay_us(uint32_t us) { (void)us; }
void osDelay(uint32_t ms) { (void)ms; }

int main(void)
{
    ADS1220_TypeDef adc = {0};
    uint32_t value = GET_ADS1220_Val(&adc);
    assert(starts == 2);
    assert(value == 0xFF800001u);
    assert(adc.Data == value);
    puts("PASS: ADS1220 discards first conversion and returns second");
    return 0;
}
'''


def run():
    with tempfile.TemporaryDirectory(prefix="adc-ads1220-test-") as directory:
        tmp = Path(directory)
        for name, text in {
            "stm32g0xx_hal.h": HAL,
            "main.h": MAIN,
            "spi.h": SPI,
            "tim.h": TIM,
            "cmsis_os.h": CMSIS,
        }.items():
            (tmp / name).write_text(text)
        source = tmp / "test_ads1220.c"
        source.write_text(HARNESS)
        executable = tmp / "test_ads1220"
        subprocess.run([
            os.environ.get("CC", "cc"), "-std=c99", "-g", "-fsanitize=address,undefined",
            "-I" + str(tmp), "-I" + str(ROOT / "Core/Inc"),
            str(source), str(ROOT / "Core/Src/ADS1220.c"), "-o", str(executable),
        ], check=True)
        subprocess.run([str(executable)], check=True)


if __name__ == "__main__":
    run()
