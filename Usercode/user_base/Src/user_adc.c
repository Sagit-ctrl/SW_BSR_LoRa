/**
  * @file    user_adc.c
  * @author  Sagit
  * @date    Mar 3, 2024
  * @brief   
  */

/* INCLUDES ------------------------------------------------------------------*/
#include "user_link.h"

/* EXTERN VARIABLES ----------------------------------------------------------*/
ADC_HandleTypeDef hadc;

/* PRIVATE MACROS AND DEFINES ------------------------------------------------*/

/* PRIVATE TYPEDEFS ----------------------------------------------------------*/

/* STATIC VARIABLES ----------------------------------------------------------*/

/* GLOBAL VARIABLES ----------------------------------------------------------*/

/* STATIC FUNCTION PROTOTYPES --- --------------------------------------------*/

/* STATIC FUNCTIONS ----------------------------------------------------------*/

/* GLOBAL FUNCTIONS ----------------------------------------------------------*/
void USER_ADC_Init(void)
{
	#if (DEVICE_TYPE_STATION == 1)
	#else
		ADC_ChannelConfTypeDef sConfig = {0};
		hadc1.Instance = ADC1;
		hadc1.Init.Resolution = ADC_RESOLUTION_12B;
		hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
		hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
		hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
		hadc1.Init.LowPowerAutoWait = DISABLE;
		hadc1.Init.ContinuousConvMode = DISABLE;
		hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
		hadc1.Init.SamplingTime = ADC_SAMPLETIME_160CYCLES_5;
		hadc1.Init.LowPowerFrequencyMode = ENABLE;
		hadc1.Init.LowPowerAutoPowerOff = DISABLE;
		hadc1.Init.DiscontinuousConvMode = DISABLE;
		hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
		hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
		hadc1.Init.DMAContinuousRequests = DISABLE;
		hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
		hadc1.Init.OversamplingMode = DISABLE;
		if (HAL_ADC_Init(&hadc1) != HAL_OK)
		{
			Error_Handler();
		}
		if (HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED) != HAL_OK)
		{
			Error_Handler();
		}
		sConfig.Channel = ADC_CHANNEL_VREFINT;
		sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
		if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
		{
			Error_Handler();
		}
	#endif
}

void HAL_ADC_MspInit(ADC_HandleTypeDef* adcHandle)
{
	#if (DEVICE_TYPE_STATION == 1)
	#else
		GPIO_InitTypeDef GPIO_InitStruct = {0};
		if(adcHandle->Instance==ADC1)
		{
			__HAL_RCC_ADC1_CLK_ENABLE();
			__HAL_RCC_GPIOA_CLK_ENABLE();
		    GPIO_InitStruct.Pin = ADC_VNA_Pin|ADC_VP_Pin|ADC_BAT_Pin;
		    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
		    GPIO_InitStruct.Pull = GPIO_NOPULL;
		    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
		}
	#endif
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef* adcHandle)
{
	#if (DEVICE_TYPE_STATION == 1)
	#else
		if(adcHandle->Instance==ADC1)
		{
			__HAL_RCC_ADC1_FORCE_RESET();
			__HAL_RCC_ADC1_RELEASE_RESET();
			__HAL_RCC_ADC1_CLK_DISABLE();
			HAL_GPIO_DeInit(ADC_VNA_GPIO_Port, ADC_VNA_Pin);
			HAL_GPIO_DeInit(ADC_VP_GPIO_Port, ADC_VP_Pin);
			HAL_GPIO_DeInit(ADC_BAT_GPIO_Port, ADC_BAT_Pin);
		}
	#endif
}

void USER_ADC_LPM(void)
{
    ADC_ChannelConfTypeDef sConfig;
    sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
    sConfig.Rank = ADC_RANK_NONE;
    HAL_ADC_ConfigChannel (&hadc1, &sConfig);
    sConfig.Channel = ADC_CHANNEL_VREFINT;
    sConfig.Rank = ADC_RANK_NONE;
    HAL_ADC_ConfigChannel (&hadc1, &sConfig);
}

uint16_t USER_ADC_Read(uint32_t Channel)
{
    ADC_ChannelConfTypeDef  sConfig;
    uint16_t adcData = 0;

    USER_ADC_Init();
	while (__HAL_PWR_GET_FLAG(PWR_FLAG_VREFINTRDY) == RESET) {};
	__HAL_RCC_ADC1_CLK_ENABLE() ;
	HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
	sConfig.Channel = ADC_CHANNEL_MASK;
	sConfig.Rank = ADC_RANK_NONE;
	HAL_ADC_ConfigChannel( &hadc1, &sConfig);
	sConfig.Channel = Channel;
	sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
	HAL_ADC_ConfigChannel( &hadc1, &sConfig);
	if (HAL_ADC_Start(&hadc) != HAL_OK)
	{
		Error_Handler();
	}
	HAL_ADC_PollForConversion( &hadc1, HAL_MAX_DELAY );
	HAL_ADC_Stop(&hadc1);
	adcData = HAL_ADC_GetValue ( &hadc1);
	 HAL_ADC_DeInit(&hadc1);
	__HAL_RCC_ADC1_CLK_DISABLE() ;
    return adcData;
}

uint16_t USER_ADC_VNA(void)
{
	uint16_t Value_ADC_u16;
	uint16_t Temp_mV = 0;
    Value_ADC_u16 = USER_ADC_Read(ADC_CHANNEL_2);
	Temp_mV = (VREF * Value_ADC_u16) / 4095 + DEFAULT_OFFSET_ADC_VNA / 2;
	return Temp_mV;
}

uint16_t USER_ADC_VP(void)
{
	uint16_t Value_ADC_u16;
	uint16_t Temp_mV = 0;
    Value_ADC_u16 = USER_ADC_Read(ADC_CHANNEL_3);
	Temp_mV = (VREF * Value_ADC_u16) / 4095 + DEFAULT_OFFSET_ADC_VP / 2;
	return Temp_mV;
}

uint8_t USER_ADC_BAT(void)
{
	uint16_t Value_ADC_u16;
	float Temp_mV = 0;
	float PercentBat;
    Value_ADC_u16 = USER_ADC_Read(ADC_CHANNEL_4);
	Temp_mV = (VREF * Value_ADC_u16) / 4095.0 / 1000.0 * 2;
	PercentBat = 579 * Temp_mV * Temp_mV - 1936 * Temp_mV + 2089 -
							55 * Temp_mV * Temp_mV * Temp_mV;
	return PercentBat;
}

/* End of file ----------------------------------------------------------------*/
