#include "sw_spi.h"

/* 简单微秒延时 */
static void SW_SPI_DelayUs(uint32_t us)
{
    for(uint32_t i = 0; i < us * 72; i++)
    {
        __NOP();
    }
}

void SW_SPI_Init(SW_SPI_HandleTypeDef *hspi)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* 开启GPIO时钟 */
//    if(hspi->SCLK_PORT == GPIOA) __HAL_RCC_GPIOA_CLK_ENABLE();
//    if(hspi->SCLK_PORT == GPIOB) __HAL_RCC_GPIOB_CLK_ENABLE();
//    if(hspi->SCLK_PORT == GPIOC) __HAL_RCC_GPIOC_CLK_ENABLE();

//    if(hspi->MOSI_PORT == GPIOA) __HAL_RCC_GPIOA_CLK_ENABLE();
//    if(hspi->MOSI_PORT == GPIOB) __HAL_RCC_GPIOB_CLK_ENABLE();
//    if(hspi->MOSI_PORT == GPIOC) __HAL_RCC_GPIOC_CLK_ENABLE();

//    if(hspi->MISO_PORT == GPIOA) __HAL_RCC_GPIOA_CLK_ENABLE();
//    if(hspi->MISO_PORT == GPIOB) __HAL_RCC_GPIOB_CLK_ENABLE();
//    if(hspi->MISO_PORT == GPIOC) __HAL_RCC_GPIOC_CLK_ENABLE();

//    if(hspi->NSS_PORT == GPIOA) __HAL_RCC_GPIOA_CLK_ENABLE();
//    if(hspi->NSS_PORT == GPIOB) __HAL_RCC_GPIOB_CLK_ENABLE();
//    if(hspi->NSS_PORT == GPIOC) __HAL_RCC_GPIOC_CLK_ENABLE();

    // SCLK 推挽输出
    GPIO_InitStruct.Pin = hspi->SCLK_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(hspi->SCLK_PORT, &GPIO_InitStruct);

    // MOSI 推挽输出
    GPIO_InitStruct.Pin = hspi->MOSI_PIN;
    HAL_GPIO_Init(hspi->MOSI_PORT, &GPIO_InitStruct);

    // MISO 上拉输入
    GPIO_InitStruct.Pin = hspi->MISO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(hspi->MISO_PORT, &GPIO_InitStruct);

    // NSS 推挽输出
    GPIO_InitStruct.Pin = hspi->NSS_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(hspi->NSS_PORT, &GPIO_InitStruct);

    // SPI Mode0 初始电平：SCLK=0，NSS高电平
    HAL_GPIO_WritePin(hspi->SCLK_PORT, hspi->SCLK_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(hspi->NSS_PORT, hspi->NSS_PIN, GPIO_PIN_SET);
}

void SW_SPI_NSS_Low(SW_SPI_HandleTypeDef *hspi)
{
    HAL_GPIO_WritePin(hspi->NSS_PORT, hspi->NSS_PIN, GPIO_PIN_RESET);
}

void SW_SPI_NSS_High(SW_SPI_HandleTypeDef *hspi)
{
    HAL_GPIO_WritePin(hspi->NSS_PORT, hspi->NSS_PIN, GPIO_PIN_SET);
}

// SPI Mode0 CPOL=0 CPHA=0 高位先行 MSB FIRST
uint8_t SW_SPI_RWByte(SW_SPI_HandleTypeDef *hspi, uint8_t txData)
{
    uint8_t rxData = 0;

    for(uint8_t i = 0; i < 8; i++)
    {
        // 输出最高位
        if(txData & 0x80)
            HAL_GPIO_WritePin(hspi->MOSI_PORT, hspi->MOSI_PIN, GPIO_PIN_SET);
        else
            HAL_GPIO_WritePin(hspi->MOSI_PORT, hspi->MOSI_PIN, GPIO_PIN_RESET);

        txData <<= 1;
        SW_SPI_DelayUs(1);

        // SCLK上升沿采样（Mode0）
        HAL_GPIO_WritePin(hspi->SCLK_PORT, hspi->SCLK_PIN, GPIO_PIN_SET);
        SW_SPI_DelayUs(1);

        // 读取MISO
        rxData <<= 1;
        if(HAL_GPIO_ReadPin(hspi->MISO_PORT, hspi->MISO_PIN))
        {
            rxData |= 0x01;
        }

        // SCLK下降沿
        HAL_GPIO_WritePin(hspi->SCLK_PORT, hspi->SCLK_PIN, GPIO_PIN_RESET);
        SW_SPI_DelayUs(1);
    }
    return rxData;
}

// SPI Mode1  CPOL=0 CPHA=1 适配 2200
//uint8_t SW_SPI_RWByte(SW_SPI_HandleTypeDef *hspi, uint8_t txData)
//{
//    uint8_t rxData = 0;

//    for(uint8_t i = 0; i < 8; i++)
//    {
//        // 1.上升沿：主机输出MOSI数据
//        HAL_GPIO_WritePin(hspi->SCLK_PORT, hspi->SCLK_PIN, GPIO_PIN_SET);
//        SW_SPI_DelayUs(1);

//        if(txData & 0x80)
//            HAL_GPIO_WritePin(hspi->MOSI_PORT, hspi->MOSI_PIN, GPIO_PIN_SET);
//        else
//            HAL_GPIO_WritePin(hspi->MOSI_PORT, hspi->MOSI_PIN, GPIO_PIN_RESET);
//        txData <<= 1;

//        // 2.下降沿：NSA2200采样MOSI，主机读取MISO
//        SW_SPI_DelayUs(1);
//        HAL_GPIO_WritePin(hspi->SCLK_PORT, hspi->SCLK_PIN, GPIO_PIN_RESET);

//        rxData <<= 1;
//        if(HAL_GPIO_ReadPin(hspi->MISO_PORT, hspi->MISO_PIN))
//        {
//            rxData |= 0x01;
//        }
//        SW_SPI_DelayUs(1);
//    }
//    return rxData;
//}

uint8_t SW_SPI_ReadByte(SW_SPI_HandleTypeDef *hspi) {
  uint8_t rxData = 0;
    for(uint8_t i = 0; i < 8; i++)
    {       

        // SCLK上升沿采样（Mode0）
        HAL_GPIO_WritePin(hspi->SCLK_PORT, hspi->SCLK_PIN, GPIO_PIN_SET);
        SW_SPI_DelayUs(1);

        // 读取MISO
        rxData <<= 1;
        if(HAL_GPIO_ReadPin(hspi->MISO_PORT, hspi->MISO_PIN))
        {
            rxData |= 0x01;
        }

        // SCLK下降沿
        HAL_GPIO_WritePin(hspi->SCLK_PORT, hspi->SCLK_PIN, GPIO_PIN_RESET);
        SW_SPI_DelayUs(1);
    }
    return rxData;
}

void SW_SPI_WriteByte(SW_SPI_HandleTypeDef *hspi, uint8_t txData)
{
    for(uint8_t i = 0; i < 8; i++)
    {
        // 输出最高位
        if(txData & 0x80)
            HAL_GPIO_WritePin(hspi->MOSI_PORT, hspi->MOSI_PIN, GPIO_PIN_SET);
        else
            HAL_GPIO_WritePin(hspi->MOSI_PORT, hspi->MOSI_PIN, GPIO_PIN_RESET);

        txData <<= 1;
        SW_SPI_DelayUs(1);

        // SCLK上升沿采样（Mode0）
        HAL_GPIO_WritePin(hspi->SCLK_PORT, hspi->SCLK_PIN, GPIO_PIN_SET);
        SW_SPI_DelayUs(1);       

        // SCLK下降沿
        HAL_GPIO_WritePin(hspi->SCLK_PORT, hspi->SCLK_PIN, GPIO_PIN_RESET);
        SW_SPI_DelayUs(1);
    }
}

void SW_SPI_SendData(SW_SPI_HandleTypeDef *hspi, uint8_t *pTxBuf, uint16_t len)
{
    for(uint16_t i = 0; i < len; i++)
    {
        SW_SPI_WriteByte(hspi, pTxBuf[i]);
    }
}

void SW_SPI_TransReceive(SW_SPI_HandleTypeDef *hspi, uint8_t *pTxBuf, uint8_t *pRxBuf, uint16_t len)
{
    for(uint16_t i = 0; i < len; i++)
    {
        pRxBuf[i] = SW_SPI_RWByte(hspi, pTxBuf[i]);
    }
}



