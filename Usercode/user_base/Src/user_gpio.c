/**
  * @file    user_gpio.c
  * @author  Sagit
  * @date    Mar 3, 2024
  * @brief   
  */

/* INCLUDES ------------------------------------------------------------------*/
#include "user_link.h"

/* EXTERN VARIABLES ----------------------------------------------------------*/

/* PRIVATE MACROS AND DEFINES ------------------------------------------------*/

/* PRIVATE TYPEDEFS ----------------------------------------------------------*/

/* STATIC VARIABLES ----------------------------------------------------------*/

/* GLOBAL VARIABLES ----------------------------------------------------------*/

/* STATIC FUNCTION PROTOTYPES --- --------------------------------------------*/

/* STATIC FUNCTIONS ----------------------------------------------------------*/

/* GLOBAL FUNCTIONS ----------------------------------------------------------*/
void USER_GPIO_PreInit(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();

	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Pin = GPIO_PIN_All & (~(GPIO_PIN_13 | GPIO_PIN_14));
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = GPIO_PIN_All;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
	HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

	__HAL_RCC_GPIOA_CLK_DISABLE();
	__HAL_RCC_GPIOB_CLK_DISABLE();
	__HAL_RCC_GPIOC_CLK_DISABLE();
	__HAL_RCC_GPIOH_CLK_DISABLE();
}

void USER_GPIO_Init(void)
{
	#if (DEVICE_TYPE_STATION == 1)
		GPIO_InitTypeDef GPIO_InitStruct = {0};

		/* GPIO Ports Clock Enable */
		__HAL_RCC_GPIOA_CLK_ENABLE();
		__HAL_RCC_GPIOB_CLK_ENABLE();

		/*Configure GPIO pin Output Level */
		HAL_GPIO_WritePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin, GPIO_PIN_RESET);

		/*Configure GPIO pin : PtPin */
		GPIO_InitStruct.Pin = LED_BLUE_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		HAL_GPIO_Init(LED_BLUE_GPIO_Port, &GPIO_InitStruct);
	#else
		GPIO_InitTypeDef GPIO_InitStruct = {0};

		__HAL_RCC_GPIOA_CLK_ENABLE();
		__HAL_RCC_GPIOB_CLK_ENABLE();

		HAL_GPIO_WritePin(GPIOB, RTC_RST_Pin|LED_GREEN_Pin|LED_BLUE_Pin, GPIO_PIN_SET);

		GPIO_InitStruct.Pin = RTC_INT_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(RTC_INT_GPIO_Port, &GPIO_InitStruct);

		GPIO_InitStruct.Pin = RTC_RST_Pin|LED_GREEN_Pin|LED_BLUE_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

		__HAL_RCC_ADC1_CLK_ENABLE();
		__HAL_RCC_GPIOA_CLK_ENABLE();
		GPIO_InitStruct.Pin = ADC_VNA_Pin|ADC_VP_Pin|ADC_BAT_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		HAL_NVIC_SetPriority(EXTI4_15_IRQn, 1, 0);
		HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
		#endif
}

void USER_GPIO_DeInit(void)
{
	#if (DEVICE_TYPE_STATION == 1)
		GPIO_InitTypeDef GPIO_InitStruct ={0};

		__HAL_RCC_GPIOA_CLK_ENABLE();
		__HAL_RCC_GPIOB_CLK_ENABLE();

		GPIO_InitStruct.Mode   = GPIO_MODE_ANALOG;
		GPIO_InitStruct.Pull   = GPIO_NOPULL;

		GPIO_InitStruct.Pin = LED_BLUE_Pin;
		HAL_GPIO_Init(LED_BLUE_GPIO_Port, &GPIO_InitStruct);
	#else
		GPIO_InitTypeDef GPIO_InitStruct ={0};

		__HAL_RCC_GPIOA_CLK_ENABLE();
		__HAL_RCC_GPIOB_CLK_ENABLE();

		GPIO_InitStruct.Mode   = GPIO_MODE_ANALOG;
		GPIO_InitStruct.Pull   = GPIO_NOPULL;

		GPIO_InitStruct.Pin = RTC_RST_Pin|LED_GREEN_Pin|LED_BLUE_Pin;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

		  /* Disable GPIOs clock */
		__HAL_RCC_GPIOA_CLK_DISABLE();
		__HAL_RCC_GPIOB_CLK_DISABLE();
	#endif
}
/* End of file ----------------------------------------------------------------*/
