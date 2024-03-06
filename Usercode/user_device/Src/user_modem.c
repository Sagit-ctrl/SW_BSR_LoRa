#include "user_rtc.h"
#include "user_modem.h"
#include "user_payload.h"
#include "user_adc.h"
#include "user_define.h"
#include "user_flash.h"
#include "radio.h"
#include "user_link.h"

/* External variables ---------------------------------------------------------*/
ST_TIME_FORMAT          sRTCSet = {0};
uint8_t aDCU_ID[MAX_DCU_ID_LENGTH] = DCUID;
uint8_t aNET_ID[MAX_DCU_ID_LENGTH] = NETID;

uint8_t aAT_CMD [LEGNTH_AT_SERIAL];
uint8_t aMULTI_MESS [LENGTH_MULTI_TX];
uint8_t aBACKUP[255];

Struct_Modem_Variable		sModem =
{
	.sDCU_id    		= {&aDCU_ID[0], 4},
	.sNET_id			= {&aNET_ID[0], 4},
	.strATCmd   		= {&aAT_CMD[0], 0},
	.sBackup			= {&aBACKUP[0], 0},
	.bNeedConfirm		= 0,
	.TimeTrySendAgain	= 0,
	.TypeDataMessage	= _DATA_NONE,
	#if (DEVICE_TYPE_STATION == 1)
		.Mode				= _MODE_SLEEP,
	#else
		.Mode				= _MODE_SLEEP,
	#endif
	.SendAll			= 0,
};

SModemFreqActionInformation     sFreqInfor =
{
    .NumWakeup_u8           = DEFAULT_NUMBER_WAKE_UP,
    .FreqWakeup_u32         = DEFAULT_FREQ_WAKEUP,
};
/* Exported functions --------------------------------------------------------*/
void Reset_Chip_Immediately (void)
{
	__disable_irq();
	NVIC_SystemReset();
}

void DCU_Response_AT(uint8_t *data, uint16_t length)
{
	HAL_UART_Transmit(&uart_debug, data, length, 1000);
	HAL_UART_Transmit(&uart_debug, (uint8_t*)"\r\n", 2, 1000);
	#if (DEVICE_TYPE_STATION == 1)
		HAL_UART_Transmit(&uart_mcu, data, length, 1000);
	#endif
}

void Init_Memory_Infor(void)
{
    Init_Timer_Send();
    Init_Device_Type();
	Init_ID(0);
	Init_ID(1);
}

/**
 * @param	0: DCU_ID
 * 			1: NET_ID
 */
void Init_ID(uint8_t type)
{
    uint8_t temp = 0;
    if(type == 0)
    {
    	temp = *(__IO uint8_t*) ADDR_DCUID;
    	if (temp != FLASH_BYTE_EMPTY)
    	{
            Flash_Get_Infor ((ADDR_DCUID + 1), sModem.sDCU_id.Data_a8, &sModem.sDCU_id.Length_u16, 10);
    	} else
    	{
    		Save_ID(type);
    	}
    } else
    {
    	temp = *(__IO uint8_t*) ADDR_NETID;
    	if (temp != FLASH_BYTE_EMPTY)
    	{
            Flash_Get_Infor ((ADDR_NETID + 1), sModem.sNET_id.Data_a8, &sModem.sNET_id.Length_u16, 10);
    	} else
    	{
    		Save_ID(type);
    	}
    }
}

/**
 * @param	0: DCU_ID
 * 			1: NET_ID
 */
void Save_ID(uint8_t type)
{
    uint8_t i=0;
	uint8_t temp_arr[40] = {0};

	if(type == 0)
	{
		temp_arr[0] = BYTE_TEMP_FIRST;
		temp_arr[1] = sModem.sDCU_id.Length_u16;

		for (i = 0;i < sModem.sDCU_id.Length_u16; i++)
			temp_arr[i+2] = sModem.sDCU_id.Data_a8[i];

		OnchipFlashPageErase(ADDR_DCUID);
		OnchipFlashWriteData(ADDR_DCUID, temp_arr, 40);
	} else
	{
		temp_arr[0] = BYTE_TEMP_FIRST;
		temp_arr[1] = sModem.sNET_id.Length_u16;

		for (i = 0;i < sModem.sNET_id.Length_u16; i++)
			temp_arr[i+2] = sModem.sNET_id.Data_a8[i];

		OnchipFlashPageErase(ADDR_NETID);
		OnchipFlashWriteData(ADDR_NETID, temp_arr, 40);
	}
}

void Init_Timer_Send (void)
{
    uint8_t     temp = 0;
    uint8_t 	Buff_temp[24] = {0};
    uint8_t 	Length = 0;

    temp = *(__IO uint8_t*) (ADDR_FREQ_ACTIVE);
    if (temp != FLASH_BYTE_EMPTY)
    {
        OnchipFlashReadData(ADDR_FREQ_ACTIVE, &Buff_temp[0], 24);
        Length = Buff_temp[1];
        if (Length < 24)
        {
            sFreqInfor.NumWakeup_u8 = Buff_temp[2];
            sFreqInfor.FreqWakeup_u32 = Buff_temp[3];
            sFreqInfor.FreqWakeup_u32 = (sFreqInfor.FreqWakeup_u32 << 8) | Buff_temp[4];
        }
    } else
    {
        Save_Freq_Send_Data();
    }
}

void Save_Freq_Send_Data (void)
{
    uint8_t Buff_temp[24] = {0};

    Buff_temp[0] = BYTE_TEMP_FIRST;
    Buff_temp[1] = 3;
    Buff_temp[2] = sFreqInfor.NumWakeup_u8;

    Buff_temp[3] = (sFreqInfor.FreqWakeup_u32 >> 8) & 0xFF;
    Buff_temp[4] = sFreqInfor.FreqWakeup_u32 & 0xFF;

    Erase_Firmware(ADDR_FREQ_ACTIVE, 1);
    OnchipFlashWriteData(ADDR_FREQ_ACTIVE, &Buff_temp[0], 16);
}

void Init_Device_Type (void)
{
    uint8_t     temp = 0;
    uint8_t 	Buff_temp[24] = {0};
    uint8_t 	Length = 0;

    temp = *(__IO uint8_t*) (ADDR_DEVICE_TYPE);
    if (temp != FLASH_BYTE_EMPTY)
    {
        OnchipFlashReadData(ADDR_DEVICE_TYPE, &Buff_temp[0], 24);
        Length = Buff_temp[1];
        if (Length < 24)
        {
            sModem.TypeModem_u8 = Buff_temp[2];
        }
    } else
    {
        Save_Device_Type();
    }
}

void Save_Device_Type (void)
{
    uint8_t Buff_temp[24] = {0};

    Buff_temp[0] = BYTE_TEMP_FIRST;
    Buff_temp[1] = 1;
    Buff_temp[2] = sModem.TypeModem_u8;

    Erase_Firmware(ADDR_DEVICE_TYPE, 1);
    OnchipFlashWriteData(ADDR_DEVICE_TYPE, &Buff_temp[0], 16);
}

/* End of file ----------------------------------------------------------------*/
