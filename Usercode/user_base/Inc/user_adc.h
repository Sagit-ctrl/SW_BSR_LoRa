/**
  * @file    user_adc.h
  * @author  Sagit
  * @date    Mar 3, 2024
  * @brief   
  */

#ifndef USER_ADC
#define USER_ADC

/* INCLUDES ------------------------------------------------------------------*/
#include "main.h"

/* MACROS AND DEFINES --------------------------------------------------------*/
#define hadc1               hadc
#define VREF				((uint32_t) 3000)

/* TYPEDEFS ------------------------------------------------------------------*/

/* EXTERNAL VARIABLES --------------------------------------------------------*/
extern ADC_HandleTypeDef hadc;

/* GLOBAL FUNCTION PROTOTYPES ------------------------------------------------*/
void USER_ADC_Init(void);
void USER_ADC_LPM(void);
uint16_t USER_ADC_Read(uint32_t Channel);
uint16_t USER_ADC_VNA(void);
uint16_t USER_ADC_VP(void);
uint8_t USER_ADC_BAT(void);

#endif /*USER_ADC*/

/* End of file ----------------------------------------------------------------*/
