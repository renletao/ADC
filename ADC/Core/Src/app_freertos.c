/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : app_freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usart.h"
#include "stdio.h"
#include "mb.h"
#include "mbutils.h"
#include <string.h>
#include "CmdHelper.h"
#include "ADS1220.h"
#include "flash.h"
#include "SoftIIC.h"
#include "tim.h"
#include "iwdg.h"
#include "wf2200.h"
#include "wf183.h"
#include "wf100s.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

#define SLAVE_ADDRESS 31

#define REG_INPUT_START 30001
#define SENDBUF_LENS 120
#define REG_HOLDING_START 40001
#define RECVBUF_LENS 64
#define RETRY_COUNT 2
#define REG_INPUT2_START 20001
uint16_t recvBuf[RECVBUF_LENS];
uint16_t sendBuf[SENDBUF_LENS]; 
uint16_t sendBufBy[SENDBUF_LENS]; 

uint8_t IsTaskDone;
uint8_t IsTask_mub;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
osThreadId defaultTaskHandle;
osThreadId myTask02Handle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
uint8_t UART485_ADDR;
osThreadId myTask03Handle;
void Sync183Task(void const * argument);
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);
void ModbusTask(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of myTask02 */
  osThreadDef(myTask02, ModbusTask, osPriorityIdle, 0, 128);
  myTask02Handle = osThreadCreate(osThread(myTask02), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
    
  /* definition and creation of myTask02 */
  osThreadDef(myTask03, Sync183Task, osPriorityNormal, 0, 256);
  myTask03Handle = osThreadCreate(osThread(myTask03), NULL);
  
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  uint8_t i=0;
  /* Infinite loop */
  for(;;)
  {      
      HAL_IWDG_Refresh(&hiwdg);
      if(i ==1 )
		HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_SET);
      else if(i ==6) {
        i = 0;
		HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_RESET);
      }
      i++;
      osDelay(500);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_ModbusTask */
/**
* @brief Function implementing the myTask02 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_ModbusTask */
void ModbusTask(void const * argument)
{
  /* USER CODE BEGIN ModbusTask */
  /* Infinite loop */
	
	
	HAL_TIM_Base_Start(&htim1);
	ST_Flash_Read(FLASH_USER_START_ADDR,20,(uint8_t*)&ADS1220);
	ADS1220_Init(&ADS1220);
	iic_init(SDA_GPIO_Port, SDA_Pin, SCL_GPIO_Port, SCL_Pin);
	UART485_ADDR = (!ADDR1_READ|!ADDR2_READ|!ADDR3_READ|!ADDR4_READ|!ADDR5_READ)+1;
	eMBInit(MB_RTU, SLAVE_ADDRESS, SLAVE_ADDRESS, 115200, MB_PAR_NONE);
	eMBEnable();
	/* Infinite loop */
	for (;;) {
		eMBPoll();
		osDelay(1);
	}
  /* USER CODE END ModbusTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void Sync183Task(void const * argument)
{
  /* USER CODE BEGIN ModbusTask */
  /* Infinite loop */
	HAL_TIM_Base_Start(&htim2);
  HAL_GPIO_WritePin(PWR_GPIO_Port, PWR_Pin, GPIO_PIN_RESET);
	/* Infinite loop */
	for (;;) {
		switch(IsTask_mub)
		{
			case WF183DE:
        if(IsTaskDone) 
        {
					memset(sendBufBy, 0, sizeof(uint16_t)* SENDBUF_LENS);
					AllTestFor183(sendBufBy, SENDBUF_LENS);            
					IsTaskDone = 0;
        }
				break;
			case WF100S:
				if(IsTaskDone) 
        {
					memset(sendBufBy, 0, sizeof(uint16_t)* SENDBUF_LENS);
					AllTestFor100S(sendBufBy, SENDBUF_LENS);        
          osDelay(1);
					IsTaskDone = 0;
        }
				break;
			case WF2200:
				if(IsTaskDone) 
        {
					memset(sendBufBy, 0, sizeof(uint16_t)* SENDBUF_LENS);
					AllTestFor2200(sendBufBy, SENDBUF_LENS);            
					IsTaskDone = 0;
        }
				break;
			default:
				
				break;
		}
		osDelay(1);
	}
  /* USER CODE END ModbusTask */
}

eMBErrorCode eMBRegInputCB(UCHAR *pucRegBuffer, USHORT usAddress, USHORT usNRegs) {
	eMBErrorCode eStatus = MB_ENOERR;
	int i = 0;
	if ((usAddress/* >=*/== REG_INPUT_START) && (usAddress + usNRegs <= REG_INPUT_START + SENDBUF_LENS)) {
		i = (int) (usAddress - REG_INPUT_START);
		while (usNRegs > 0) {
			*pucRegBuffer++ = (uint8_t) (sendBuf[i] >> 8);
			*pucRegBuffer++ = (uint8_t) (sendBuf[i] & 0xFF);
			i++;
			usNRegs--;
		}
	}
	/*else if ((usAddress >= REG_INPUT2_START) && (usAddress + usNRegs <= REG_INPUT2_START + DATABUF_LENS)) {
	 i = (int) (usAddress - REG_INPUT2_START);
	 while (usNRegs > 0) {
	 *pucRegBuffer++ = (uint8_t) (dataBuf[i] >> 8);
	 *pucRegBuffer++ = (uint8_t) (dataBuf[i] & 0xFF);
	 i++;
	 usNRegs--;
	 }
	 } */else {
		eStatus = MB_ENOREG;
	}
	return eStatus;
}

eMBErrorCode eMBRegHoldingCB(UCHAR *pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode) {
	eMBErrorCode eStatus = MB_ENOERR;
	int i = 0;
	if ((usAddress /*>=*/== REG_HOLDING_START) && (usAddress + usNRegs <= REG_HOLDING_START + RECVBUF_LENS)) {
		i = (int) (usAddress - REG_HOLDING_START);
		if (eMode == MB_REG_READ) {
			while (usNRegs > 0) {
				*pucRegBuffer++ = (UCHAR) (recvBuf[i] >> 8);
				*pucRegBuffer++ = (UCHAR) (recvBuf[i] & 0xFF);
				i++;
				usNRegs--;
			}
		} else if (eMode == MB_REG_WRITE) {
			while (usNRegs > 0) {
				recvBuf[i] = *pucRegBuffer++ << 8;
				recvBuf[i] |= *pucRegBuffer++;
				i++;
				usNRegs--;
			}
			memset(sendBuf, 0, sizeof(uint16_t)* SENDBUF_LENS);
			Parse(recvBuf, i, sendBuf, SENDBUF_LENS);
		}
	} else {
		eStatus = MB_ENOREG;
	}
	return eStatus;
}

eMBErrorCode eMBRegCoilsCB(UCHAR *pucRegBuffer, USHORT usAddress, USHORT usNCoils, eMBRegisterMode eMode) {
	return MB_ENOERR;
}

eMBErrorCode eMBRegDiscreteCB(UCHAR *pucRegBuffer, USHORT usAddress, USHORT usNDiscrete) {
	return MB_ENOERR;
}

/* USER CODE END Application */

