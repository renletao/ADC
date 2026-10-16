#ifndef __SW_SPI_H
#define __SW_SPI_H

#include "main.h"

/* SPI结构体：保存一组模拟SPI的IO信息 */
typedef struct
{
    GPIO_TypeDef *SCLK_PORT;
    uint16_t      SCLK_PIN;

    GPIO_TypeDef *MOSI_PORT;
    uint16_t      MOSI_PIN;

    GPIO_TypeDef *MISO_PORT;
    uint16_t      MISO_PIN;

    GPIO_TypeDef *NSS_PORT;
    uint16_t      NSS_PIN;
} SW_SPI_HandleTypeDef;

/**
 * @brief  初始化软件SPI IO
 * @param  hspi: 自定义SPI句柄结构体指针
 */
void SW_SPI_Init(SW_SPI_HandleTypeDef *hspi);

/**
 * @brief  SPI片选拉低
 */
void SW_SPI_NSS_Low(SW_SPI_HandleTypeDef *hspi);

/**
 * @brief  SPI片选拉高
 */
void SW_SPI_NSS_High(SW_SPI_HandleTypeDef *hspi);

/**
 * @brief  读写1字节SPI（Mode0 CPOL=0 CPHA=0）
 * @retval 收到的数据
 */
uint8_t SW_SPI_RWByte(SW_SPI_HandleTypeDef *hspi, uint8_t txData);

/**
 * @brief  连续发送多个字节
 */
void SW_SPI_SendData(SW_SPI_HandleTypeDef *hspi, uint8_t *pTxBuf, uint16_t len);

/**
 * @brief  连续收发数据
 */
void SW_SPI_TransReceive(SW_SPI_HandleTypeDef *hspi, uint8_t *pTxBuf, uint8_t *pRxBuf, uint16_t len);


uint8_t SW_SPI_ReadByte(SW_SPI_HandleTypeDef *hspi);
void SW_SPI_WriteByte(SW_SPI_HandleTypeDef *hspi, uint8_t txData);
#endif

