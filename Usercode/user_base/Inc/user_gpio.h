/**
  * @file    user_gpio.h
  * @author  Sagit
  * @date    Mar 3, 2024
  * @brief   
  */

#ifndef USER_GPIO
#define USER_GPIO

/* INCLUDES ------------------------------------------------------------------*/
#include "stm32l0xx.h"

/* MACROS AND DEFINES --------------------------------------------------------*/

/* TYPEDEFS ------------------------------------------------------------------*/

/* EXTERNAL VARIABLES --------------------------------------------------------*/

/* GLOBAL FUNCTION PROTOTYPES ------------------------------------------------*/
void USER_GPIO_PreInit(void);
void USER_GPIO_Init(void);
void USER_GPIO_DeInit(void);

#endif /*USER_GPIO*/

/* End of file ----------------------------------------------------------------*/
