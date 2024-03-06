/**
  * @file    user_uart.h
  * @author  Sagit
  * @date    Mar 3, 2024
  * @brief   
  */

#ifndef USER_UART
#define USER_UART

/* INCLUDES ------------------------------------------------------------------*/
#include "main.h"
#include "user_link.h"

/* MACROS AND DEFINES --------------------------------------------------------*/
#if (DEVICE_TYPE_STATION == 1)
	#define uart_debug	    huart2
	#define uart_mcu	    huart1
#else
	#define uart_debug      huart1
#endif

/* TYPEDEFS ------------------------------------------------------------------*/

/* EXTERNAL VARIABLES --------------------------------------------------------*/
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;

extern uint8_t 	UartDebugBuff[1200];
extern sData sUartDebug;

/* GLOBAL FUNCTION PROTOTYPES ------------------------------------------------*/
void USER_USART1_Init(void);
void USER_USART2_Init(void);
void USER_UART_Init_Interrupt(void);

#endif /*USER_UART*/

/* End of file ----------------------------------------------------------------*/
