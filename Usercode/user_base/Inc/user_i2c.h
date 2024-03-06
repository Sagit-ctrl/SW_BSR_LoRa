/**
  * @file    user_i2c.h
  * @author  Sagit
  * @date    Mar 3, 2024
  * @brief   
  */

#ifndef USER_I2C
#define USER_I2C

/* INCLUDES ------------------------------------------------------------------*/
#include "main.h"

/* MACROS AND DEFINES --------------------------------------------------------*/

/* TYPEDEFS ------------------------------------------------------------------*/

/* EXTERNAL VARIABLES --------------------------------------------------------*/
extern I2C_HandleTypeDef hi2c1;

/* GLOBAL FUNCTION PROTOTYPES ------------------------------------------------*/
void USER_I2C1_Init(void);

#endif /*USER_I2C*/

/* End of file ----------------------------------------------------------------*/
