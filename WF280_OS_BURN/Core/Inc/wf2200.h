#ifndef __WF2200_H__
#define __WF2200_H__

#include "main.h"

uint8_t WF2200_GetChipData( uint8_t command, uint8_t *readData, uint8_t readNum, uint8_t retryCount );
uint8_t WF2200_GetData( uint8_t *readData, uint8_t readNum);
uint8_t WF2200_GetRawData( uint8_t *readData, uint8_t readNum);
void WF2200_SPI_GetRawData(uint8_t *readData, uint8_t readNum);
uint8_t WF2200_SetOffset( uint8_t offset);
uint8_t WF5808_GetData( uint8_t *readData );
#endif //__WF2200_H__

