#include "user_main.h"
#include "user_payload.h"
#include "user_define.h"
#include "radio.h"

#include "user_link.h"
/* Exported functions --------------------------------------------------------*/
void SysApp_Init (void)
{
	USER_GPIO_Init();

	#if (DEVICE_TYPE_STATION == 1)
		USER_USART1_Init();
		USER_USART2_Init();
	#else
		USER_USART1_Init();
		USER_ADC_Init();
		USER_I2C1_Init();
	#endif

	UTIL_TIMER_Init();
	Sx_Board_Bus_Init();
	Sx_Board_IoInit();
	AppLora_Init ();
	USER_UART_Init_Interrupt();
	Init_Memory_Infor();
	AppCom_Init();
}

void SysApp_Start(void)
{
	LOG(LOG_DEBUG, "Main task start");
	UTIL_TIMER_Create(&TimerLoraTx,  0xFFFFFFFFU, UTIL_TIMER_ONESHOT, _Cb_Active_Lora_Tx_Event, NULL);
	UTIL_TIMER_SetPeriod (&TimerLoraTx, sFreqInfor.FreqWakeup_u32 * 1000);
	#if (DEVICE_TYPE_STATION == 1)
		LED_OFF(__LED_MODE);
		Radio.Rx(RX_TIMEOUT_VALUE);
	#else
		LED_OFF(__LED_MODE);
		LED_OFF(__LED_MEASURE);
		UTIL_TIMER_Start (&TimerLoraTx);
		USER_Payload_Node_Single(sModem.TimeDelaySingle_u32);
	#endif
}

void Main_Task (void)
{
    uint8_t TaskStatus_u8 = 0;

    SysApp_Init();
    SysApp_Start();
	for (;;)
	{
		TaskStatus_u8 = 0;
		TaskStatus_u8 |= AppCom_Task();
		TaskStatus_u8 |= AppLora_Task();
	}
}
