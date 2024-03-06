/**
  * @file    user_uart.c
  * @author  Sagit
  * @date    Mar 3, 2024
  * @brief   
  */

/* INCLUDES ------------------------------------------------------------------*/
#include "user_link.h"

/* EXTERN VARIABLES ----------------------------------------------------------*/
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

uint8_t UartDebugBuff[1200] = {0};
sData 	sUartDebug = {&UartDebugBuff[0], 0};
/* PRIVATE MACROS AND DEFINES ------------------------------------------------*/

/* PRIVATE TYPEDEFS ----------------------------------------------------------*/

/* STATIC VARIABLES ----------------------------------------------------------*/

/* GLOBAL VARIABLES ----------------------------------------------------------*/

/* STATIC FUNCTION PROTOTYPES --- --------------------------------------------*/

/* STATIC FUNCTIONS ----------------------------------------------------------*/

/* GLOBAL FUNCTIONS ----------------------------------------------------------*/
void USER_USART1_Init(void)
{
	huart1.Instance = USART1;
	huart1.Init.BaudRate = 115200;
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.Parity = UART_PARITY_NONE;
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	if (HAL_UART_Init(&huart1) != HAL_OK)
	{
		Error_Handler();
	}
}

void USER_USART2_Init(void)
{
	huart2.Instance = USART2;
	huart2.Init.BaudRate = 115200;
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.Mode = UART_MODE_TX_RX;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.OverSampling = UART_OVERSAMPLING_16;
	huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	if (HAL_UART_Init(&huart2) != HAL_OK)
	{
		Error_Handler();
	}
}

void HAL_UART_MspInit(UART_HandleTypeDef *uartHandle)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
	if(uartHandle->Instance==USART1)
	{
		__HAL_RCC_USART1_CLK_ENABLE();
		__HAL_RCC_GPIOA_CLK_ENABLE();

		PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART1;
		PeriphClkInitStruct.Usart2ClockSelection = RCC_USART1CLKSOURCE_SYSCLK;
		if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
		{
			Error_Handler();
		}
		/**USART1 GPIO Configuration
		PA9     ------> USART1_TX
		PA10     ------> USART1_RX
		*/
		#if (DEVICE_TYPE_STATION == 1)
				GPIO_InitStruct.Pin = MCU_TX_Pin|MCU_RX_Pin;
		#else
				GPIO_InitStruct.Pin = DEBUG_TX_Pin|DEBUG_RX_Pin;
		#endif
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF4_USART1;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		HAL_NVIC_SetPriority(USART1_IRQn, 0, 1);
		HAL_NVIC_EnableIRQ(USART1_IRQn);
	}
	else if (uartHandle->Instance == USART2)
	{
		__HAL_RCC_USART2_CLK_ENABLE();
		__HAL_RCC_GPIOA_CLK_ENABLE();

		PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART2;
		PeriphClkInitStruct.Usart2ClockSelection = RCC_USART2CLKSOURCE_SYSCLK;
		if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
		{
			Error_Handler();
		}
		/**USART2 GPIO Configuration
		PA2     ------> USART2_TX
		PA3     ------> USART2_RX
		*/
		GPIO_InitStruct.Pin = DEBUG_TX_Pin|DEBUG_RX_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF4_USART2;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		HAL_NVIC_SetPriority(USART2_IRQn, 0, 1);
		HAL_NVIC_EnableIRQ(USART2_IRQn);
	}
}

void HAL_UART_MspDeInit(UART_HandleTypeDef *uartHandle)
{
	if(uartHandle->Instance==USART1)
	{
		__HAL_RCC_USART1_CLK_DISABLE();
		/**USART1 GPIO Configuration
		PA9     ------> USART1_TX
		PA10     ------> USART1_RX
		*/
		HAL_GPIO_DeInit(GPIOA, DEBUG_TX_Pin|DEBUG_RX_Pin);
		HAL_NVIC_DisableIRQ(USART1_IRQn);
	}
	else if (uartHandle->Instance == USART2)
	{
		__HAL_RCC_USART2_CLK_DISABLE();
		/**USART2 GPIO Configuration
		PA2     ------> USART2_TX
		PA3     ------> USART2_RX
		*/
		HAL_GPIO_DeInit(GPIOA, DEBUG_TX_Pin|DEBUG_RX_Pin);
		HAL_NVIC_DisableIRQ(USART2_IRQn);
	}
}

void USER_UART_Init_Interrupt(void)
{
	#if (DEVICE_TYPE_STATION == 1)
		__HAL_UART_ENABLE_IT(&uart_debug, UART_IT_RXNE);
		__HAL_UART_ENABLE_IT(&uart_mcu, UART_IT_RXNE);
	#else
		__HAL_UART_ENABLE_IT(&uart_debug, UART_IT_RXNE);
	#endif
}
/* End of file ----------------------------------------------------------------*/
