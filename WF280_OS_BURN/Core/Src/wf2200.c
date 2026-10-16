
#include "wf2200.h"
#include "SoftIIC.h"
#include "cmsis_os.h"
#include "string.h"
#include "sw_spi.h"

#define WF2200IIC_ADDRESS 0xDA

SW_SPI_HandleTypeDef spi_dev1 = {
    .SCLK_PORT = IIC_SCL_GPIO_Port,
    .SCLK_PIN  = IIC_SCL_Pin,
    .MOSI_PORT = IIC_SDA_GPIO_Port,
    .MOSI_PIN  = IIC_SDA_Pin,
    .MISO_PORT = USART_RX_GPIO_Port,
    .MISO_PIN  = USART_RX_Pin,
    .NSS_PORT  = USART_TX_GPIO_Port,
    .NSS_PIN   = USART_TX_Pin,
};

uint8_t I2C_2200WriteBytes(uint8_t command, const uint8_t *para, uint8_t paraNum) {
	uint8_t i = 0;
	uint8_t err = 0;
	do {
		iic_start();
		iic_sendbyte(WF2200IIC_ADDRESS | IIC_WRITE);
		if (iic_wait_ack() != 0){
			err = 1;
			break;
		}

		iic_sendbyte(command);
		if (iic_wait_ack() != 0){
			err = 1;
			break;
		}

		for (i = 0; i < paraNum; i++) {
			iic_sendbyte(para[i]);
			if (iic_wait_ack() != 0){
				err = 1;
				break;
			}
		}

	} while (0);
	iic_stop();
	//iic_idle();
	return err;
}

uint8_t I2C_2200ReadBytes( uint8_t *readData, uint8_t readNum) {
	uint8_t i = 0;
	uint8_t err = 0;
	do {
		iic_start();
		iic_sendbyte(WF2200IIC_ADDRESS | IIC_READ);
		if (iic_wait_ack() != 0){
			err = 1;
			break;
		}

		for (i = 0; i < readNum; i++) {
			readData[i] = 0;
			if (i != (readNum - 1)) {
				readData[i] = iic_readbyte(1);
			} else {
				readData[i] = iic_readbyte(0);
			}
		}

	} while (0);
	iic_stop();
	//iic_idle();
	return err;
}


uint8_t WF2200_GetChipData( uint8_t command, uint8_t *readData, uint8_t readNum, uint8_t retryCount ) {
	uint8_t err = 0;
  if(readNum > 0) memset(readData, 0, (readNum * sizeof(uint8_t)));
	do {		
		if ((err = I2C_2200WriteBytes( command, NULL, 0)) != 0)
			break;
    osDelay(2);
		if ((err = I2C_2200ReadBytes(readData, readNum)) != 0)
			break;
    else break;
	} while (--retryCount>0);
	return err;
}

uint8_t WF2200_GetData( uint8_t *readData, uint8_t readNum) {

  uint8_t err = 0;
  uint8_t buf[2];
  if(readNum > 0) memset(readData, 0, (readNum * sizeof(uint8_t)));
	do {		
    buf[0] = 0x0A;
		if ((err = I2C_2200WriteBytes( 0x30, buf, 1)) != 0)
			break;
    osDelay(10);
    if ((err = I2C_2200WriteBytes( 0x06, NULL, 0)) != 0)
			break;
    osDelay(2);
		if ((err = I2C_2200ReadBytes(readData, readNum)) != 0)
			break;
    else break;
	} while (0);
	return err;
}

uint8_t WF2200_GetRawData( uint8_t *readData, uint8_t readNum) {

  uint8_t err = 0;
  uint8_t buf[3];
  if(readNum > 0) memset(readData, 0, (readNum * sizeof(uint8_t)));
	do {		
    buf[0] = 17;
    buf[1] = 40;
    buf[2] = 192;
    if ((err = I2C_2200WriteBytes( 0xA5, buf, 3)) != 0)
			break;
    osDelay(10);
    buf[0] = 0x08;
		if ((err = I2C_2200WriteBytes( 0x30, buf, 1)) != 0)
			break;
    osDelay(5);
    if ((err = I2C_2200WriteBytes( 0x09, NULL, 0)) != 0)
			break;
    osDelay(2);
		if ((err = I2C_2200ReadBytes(&readData[3], 2)) != 0)
			break;
    buf[0] = 19;
    if ((err = I2C_2200WriteBytes( 0xA5, buf, 1)) != 0)
			break;
    osDelay(10);
    buf[0] = 0x09;
		if ((err = I2C_2200WriteBytes( 0x30, buf, 1)) != 0)
			break;
    osDelay(5);
    if ((err = I2C_2200WriteBytes( 0x06, NULL, 0)) != 0)
			break;
    osDelay(2);
		if ((err = I2C_2200ReadBytes(readData, 3)) != 0)
			break;
    else break;
	} while (0);
	return err;
}

uint8_t WF2200_SetOffset( uint8_t offset) {

  uint8_t err = 0;
  uint8_t buf[3];
	do {		
    //¼ì²â
    if ((err = I2C_2200WriteBytes( 0xA8, NULL, 0)) != 0)
			break;
    if ((err = I2C_2200ReadBytes(buf, 1)) != 0)
			break;
    if(buf[0] != 0) {
      err = 0xEF;
      break;
    }
    //Ð´offset
    buf[0] = offset;
    if ((err = I2C_2200WriteBytes( 0xA8, buf, 1)) != 0)
			break;
    osDelay(5);
    //otp
    buf[0] = 0xC2;
		if ((err = I2C_2200WriteBytes( 0x6B, buf, 1)) != 0)
			break;
    osDelay(10);
    buf[0] = 0x0D;
    if ((err = I2C_2200WriteBytes( 0x30, buf, 1)) != 0)
			break;
    osDelay(10);
    buf[0] = 0x6A;
		if ((err = I2C_2200WriteBytes( 0x6C, buf, 1)) != 0)
			break;
    osDelay(300);
    buf[0] = 0x00;
		if ((err = I2C_2200WriteBytes( 0x30, buf, 1)) != 0)
			break;
	} while (0);
	return err;
}

void WF2200_SPI_WriteReg(uint8_t addr, uint8_t value){
  SW_SPI_NSS_Low(&spi_dev1);
  SW_SPI_WriteByte(&spi_dev1,0x00);
  SW_SPI_WriteByte(&spi_dev1,addr);
  SW_SPI_WriteByte(&spi_dev1,value);
  SW_SPI_NSS_High(&spi_dev1);
}

uint8_t WF2200_SPI_ReadReg(uint8_t addr){
  uint8_t value;
  SW_SPI_NSS_Low(&spi_dev1);
  SW_SPI_WriteByte(&spi_dev1,0x80);//0x80:1 byte£¬0xA0:2 bytes£¬0xC0:3 bytes£¬0xE0£ºother
  SW_SPI_WriteByte(&spi_dev1,addr);
  value = SW_SPI_ReadByte(&spi_dev1);
  SW_SPI_NSS_High(&spi_dev1);
  return value;
}

void WF2200_SPI_GetRawData(uint8_t *readData, uint8_t readNum) {
  
  if(readNum < 5) return;
  
  SW_SPI_Init(&spi_dev1);
  osDelay(5);
  
  WF2200_SPI_WriteReg(0x00, 0x01);//0: 3-wire SPI, 1: 4-wire SPI
//  WF2200_SPI_WriteReg(0xA5, 0x11);//raw data
//  WF2200_SPI_WriteReg(0xA6, 0x28);
//  WF2200_SPI_WriteReg(0xA7, 0xC0);
//  osDelay(5);
  WF2200_SPI_WriteReg(0x30, 0x08);
  osDelay(5);
  readData[3] = WF2200_SPI_ReadReg(0x09);
  readData[4] = WF2200_SPI_ReadReg(0x0A);
//  WF2200_SPI_WriteReg(0xA5, 0x13);//cal data
//  osDelay(5);
  WF2200_SPI_WriteReg(0x30, 0x09);
  osDelay(5);
  readData[0] = WF2200_SPI_ReadReg(0x06);
  readData[1] = WF2200_SPI_ReadReg(0x07);
  readData[2] = WF2200_SPI_ReadReg(0x08);
}

uint8_t WF5808_GetData( uint8_t *readData ) {

  uint8_t i;
  uint8_t err = 0;
  uint8_t buf[2];
	do {		
    //otp flag: 0x0AÐ´0x19
    buf[0] = 0x19;
		if ((err = I2C_2200WriteBytes( 0x0A, buf, 1)) != 0)
			break;    
    osDelay(20);    
    if ((err = I2C_2200WriteBytes( 0x0C, NULL, 0)) != 0)
      break;
		if ((err = I2C_2200ReadBytes(readData, 1)) != 0)
			break;
    //id: 0x0AÐ´0x03
    buf[0] = 0x03;
		if ((err = I2C_2200WriteBytes( 0x0A, buf, 1)) != 0)
			break;    
    osDelay(20);    
    if ((err = I2C_2200WriteBytes( 0x0C, NULL, 0)) != 0)
      break;
		if ((err = I2C_2200ReadBytes(&readData[1], 4)) != 0)
			break;
    //calT: 0x0AÐ´0x04
    buf[0] = 0x04;
		if ((err = I2C_2200WriteBytes( 0x0A, buf, 1)) != 0)
			break;    
    osDelay(150);    
    if ((err = I2C_2200WriteBytes( 0x0C, NULL, 0)) != 0)
      break;
		if ((err = I2C_2200ReadBytes(&readData[5], 4)) != 0)
			break;
    //calP: 0x0AÐ´0x06
    buf[0] = 0x06;
		if ((err = I2C_2200WriteBytes( 0x0A, buf, 1)) != 0)
			break;    
    osDelay(150);    
    if ((err = I2C_2200WriteBytes( 0x0C, NULL, 0)) != 0)
      break;
		if ((err = I2C_2200ReadBytes(&readData[9], 4)) != 0)
			break;
	} while (0);
	return err;
}