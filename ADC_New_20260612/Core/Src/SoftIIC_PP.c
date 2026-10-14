#include "SoftIIC_PP.h"

static GPIO_TypeDef *pp_sda_port;
static GPIO_TypeDef *pp_scl_port;
static uint16_t pp_sda_pin;
static uint16_t pp_scl_pin;

static void pp_sda_out(void)
{
    GPIO_InitTypeDef init = {0};
    HAL_GPIO_WritePin(pp_sda_port, pp_sda_pin, GPIO_PIN_SET);
    init.Pin = pp_sda_pin;
    init.Mode = GPIO_MODE_OUTPUT_PP;
    init.Pull = GPIO_PULLUP;
    init.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(pp_sda_port, &init);
}

static void pp_sda_in(void)
{
    GPIO_InitTypeDef init = {0};
    init.Pin = pp_sda_pin;
    init.Mode = GPIO_MODE_INPUT;
    init.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(pp_sda_port, &init);
}

static void pp_scl_out(void)
{
    GPIO_InitTypeDef init = {0};
    HAL_GPIO_WritePin(pp_scl_port, pp_scl_pin, GPIO_PIN_SET);
    init.Pin = pp_scl_pin;
    init.Mode = GPIO_MODE_OUTPUT_PP;
    init.Pull = GPIO_PULLUP;
    init.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(pp_scl_port, &init);
}

void pp_iic_init(GPIO_TypeDef *sda_port, uint16_t sda_pin,
                 GPIO_TypeDef *scl_port, uint16_t scl_pin)
{
    pp_sda_port = sda_port;
    pp_scl_port = scl_port;
    pp_sda_pin = sda_pin;
    pp_scl_pin = scl_pin;
    pp_scl_out();
    pp_sda_out();
}

void pp_iic_start(void)
{
    pp_scl_out();
    pp_sda_out();
    HAL_GPIO_WritePin(pp_sda_port, pp_sda_pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(pp_scl_port, pp_scl_pin, GPIO_PIN_SET);
    delay_us(20);
    HAL_GPIO_WritePin(pp_sda_port, pp_sda_pin, GPIO_PIN_RESET);
    delay_us(20);
    HAL_GPIO_WritePin(pp_scl_port, pp_scl_pin, GPIO_PIN_RESET);
    delay_us(10);
}

void pp_iic_stop(void)
{
    pp_sda_out();
    HAL_GPIO_WritePin(pp_scl_port, pp_scl_pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(pp_sda_port, pp_sda_pin, GPIO_PIN_RESET);
    delay_us(20);
    HAL_GPIO_WritePin(pp_scl_port, pp_scl_pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(pp_sda_port, pp_sda_pin, GPIO_PIN_SET);
    delay_us(20);
}

void pp_iic_sendbyte(uint8_t byte)
{
    uint8_t i;
    pp_sda_out();
    for (i = 0; i < 8; ++i) {
        HAL_GPIO_WritePin(pp_sda_port, pp_sda_pin,
                         (byte & 0x80u) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        byte <<= 1;
        delay_us(5);
        HAL_GPIO_WritePin(pp_scl_port, pp_scl_pin, GPIO_PIN_SET);
        delay_us(10);
        HAL_GPIO_WritePin(pp_scl_port, pp_scl_pin, GPIO_PIN_RESET);
        delay_us(5);
    }
}

uint8_t pp_iic_wait_ack(void)
{
    uint16_t timeout = 0;
    pp_sda_in();
    HAL_GPIO_WritePin(pp_scl_port, pp_scl_pin, GPIO_PIN_SET);
    while (HAL_GPIO_ReadPin(pp_sda_port, pp_sda_pin) == GPIO_PIN_SET) {
        if (++timeout > 250u) {
            HAL_GPIO_WritePin(pp_scl_port, pp_scl_pin, GPIO_PIN_RESET);
            return 1;
        }
        delay_us(1);
    }
    HAL_GPIO_WritePin(pp_scl_port, pp_scl_pin, GPIO_PIN_RESET);
    return 0;
}

void pp_iic_ack(void)
{
    pp_sda_out();
    HAL_GPIO_WritePin(pp_sda_port, pp_sda_pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(pp_scl_port, pp_scl_pin, GPIO_PIN_SET);
    delay_us(10);
    HAL_GPIO_WritePin(pp_scl_port, pp_scl_pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(pp_sda_port, pp_sda_pin, GPIO_PIN_SET);
}

void pp_iic_nack(void)
{
    pp_sda_out();
    HAL_GPIO_WritePin(pp_sda_port, pp_sda_pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(pp_scl_port, pp_scl_pin, GPIO_PIN_SET);
    delay_us(10);
    HAL_GPIO_WritePin(pp_scl_port, pp_scl_pin, GPIO_PIN_RESET);
}

uint8_t pp_iic_readbyte(uint8_t ack)
{
    uint8_t i;
    uint8_t value = 0;
    pp_sda_in();
    for (i = 0; i < 8; ++i) {
        HAL_GPIO_WritePin(pp_scl_port, pp_scl_pin, GPIO_PIN_SET);
        delay_us(10);
        value = (uint8_t)((value << 1) |
                 (HAL_GPIO_ReadPin(pp_sda_port, pp_sda_pin) == GPIO_PIN_SET));
        HAL_GPIO_WritePin(pp_scl_port, pp_scl_pin, GPIO_PIN_RESET);
        delay_us(10);
    }
    if (ack) pp_iic_ack(); else pp_iic_nack();
    return value;
}
