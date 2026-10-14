/*
 * I2CHelper.c
 *
 *  Created on: 2020年8月4日
 *      Author: ITry
 */
#include "I2CHelper.h"
#include <string.h>
#include "cmsis_os.h"

#define WF2200IIC_ADDRESS 0xDA

void I2C_IDLE(){
	iic_idle();
}

/*
 * 杩炵画璇籒涓瘎瀛樺櫒鐨勫��
 */
uint8_t I2C_ReadNByte(I2CInfo info, uint16_t *readData) {
	uint8_t i = 0;
	uint8_t err = 0;
	if(info.data_lens > 0) memset(readData, 0, (info.data_lens * sizeof(uint8_t)));

	do {
		iic_start();
		iic_sendbyte(info.bus_addr | IIC_READ);
		err = iic_wait_ack();
		if(err) break;
		for (i = 0; i < info.data_lens; i++) {
			if (i != (info.data_lens - 1)) {
				readData[i] = iic_readbyte(1); // 璇诲畬闈炴渶鍚庝竴涓瓧鑺傚悗锛� 浜х敓搴旂瓟淇″彿
			} else {
				readData[i] = iic_readbyte(0); // 鏈�鍚庝竴涓瓧鑺傝瀹屽悗 浜х敓闈炲簲绛斾俊鍙�,浠ユ彁绀篠lave璁惧鍑嗗鎺ユ敹Stop淇″彿
			}
		}
	} while (0);
	iic_stop();
	//iic_idle();
	return err;
}

/*
 * 杩炵画鍐橬涓瘎瀛樺櫒鐨勫��
 */
uint8_t I2C_WriteNByte(I2CInfo info, const uint8_t *writeData) {
	uint8_t i = 0;
	uint8_t err = 0;

	do {
		iic_start();
		iic_sendbyte(info.bus_addr | IIC_WRITE);
		err = iic_wait_ack();
		if(err) break;
		
		while (i < info.data_lens) {
			iic_sendbyte(writeData[i]);
			err = iic_wait_ack();
			if(err) break;
			i++;
		}
	} while (0);
	iic_stop();
	//iic_idle();
	return err;
}


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




//////////////////////////////////////////////////////PP//////////////////////////////////////////////////////

uint8_t PP_I2C_2200WriteBytes(uint8_t command, const uint8_t *para, uint8_t paraNum) {
	uint8_t i = 0;
	uint8_t err = 0;
	do {
		pp_iic_start();
		pp_iic_sendbyte(WF2200IIC_ADDRESS | IIC_WRITE);
		if (pp_iic_wait_ack() != 0){
			err = 1;
			break;
		}

		pp_iic_sendbyte(command);
		if (pp_iic_wait_ack() != 0){
			err = 1;
			break;
		}

		for (i = 0; i < paraNum; i++) {
			pp_iic_sendbyte(para[i]);
			if (pp_iic_wait_ack() != 0){
				err = 1;
				break;
			}
		}

	} while (0);
	pp_iic_stop();
	//iic_idle();
	return err;
}

uint8_t PP_I2C_2200ReadBytes( uint8_t *readData, uint8_t readNum) {
	uint8_t i = 0;
	uint8_t err = 0;
	do {
		pp_iic_start();
		pp_iic_sendbyte(WF2200IIC_ADDRESS | IIC_READ);
		if (pp_iic_wait_ack() != 0){
			err = 1;
			break;
		}

		for (i = 0; i < readNum; i++) {
			readData[i] = 0;
			if (i != (readNum - 1)) {
				readData[i] = pp_iic_readbyte(1);
			} else {
				readData[i] = pp_iic_readbyte(0);
			}
		}

	} while (0);
	pp_iic_stop();
	//iic_idle();
	return err;
}


uint8_t PP_WF2200_GetData( uint8_t *readData, uint8_t readNum) {

  uint8_t err = 0;
  uint8_t buf[2];
  if(readNum > 0) memset(readData, 0, (readNum * sizeof(uint8_t)));
	do {		
    buf[0] = 0x0A;
		if ((err = PP_I2C_2200WriteBytes( 0x30, buf, 1)) != 0)
			break;
    osDelay(10);
    if ((err = PP_I2C_2200WriteBytes( 0x06, NULL, 0)) != 0)
			break;
    osDelay(2);
		if ((err = PP_I2C_2200ReadBytes(readData, readNum)) != 0)
			break;
    else break;
	} while (0);
	return err;
}




