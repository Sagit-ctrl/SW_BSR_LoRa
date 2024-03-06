#include "user_internal_mem.h"

/* Exported functions --------------------------------------------------------*/
/**
 * @brief	Kiem tra xem co bang tin moi khong
 * @retval	1: Empty
 * 			0: Not empty
 */
uint8_t Flash_Check_Have_Packet (uint8_t IndexSend, uint8_t IndexSave)
{
    if (IndexSend != IndexSave)
    	return TRUE;
    else
    	return FALSE;
}

/**
 * @brief	Save Index Send or Save
 */
void Flash_Save_U16 (uint32_t andress, uint16_t Num)
{
    uint8_t aTemp[4] = {0};
    aTemp[0] = (uint8_t) (Num >> 8);
    aTemp[1] = (uint8_t) Num;
    Save_Array(andress, &aTemp[0], 2);
}

/**
 * @brief	Read Record from Flash
 * @param	Addr
 *	 	 	*str: save data
 *          IndexMess in many packet: 1, 2,3...helpfull decode in server
 * @retval
 */
uint8_t Flash_Read_Record (uint32_t andress, sData* str, uint8_t IndexMess)
{
    uint8_t     i = 0, Length = 0;
    uint8_t		ChecksumByte = 0;
    uint8_t     FirstByte = 0;
    uint8_t     aTEMPData[256] = {0};
    FirstByte = *(__IO uint8_t*)(andress);
    Length = *(__IO uint8_t*)(andress + 1);
    if ((Length >= 255) ||(FirstByte != 0xAA))
        return 0;
    //Doc ra data
    OnchipFlashReadData(andress + 2, &aTEMPData[0], Length);
    //check crc
	for(i = 0; i < (Length-1); i++)
		ChecksumByte ^= aTEMPData[i];
    if (ChecksumByte == aTEMPData[Length-1])
    {
        *(str->Data_a8 + str->Length_u16++) = IndexMess;
        *(str->Data_a8 + str->Length_u16++) = Length;
        for(i = 0; i < Length; i++)
            *(str->Data_a8 + str->Length_u16++) = aTEMPData[i];
        return 1;
    }
    return 0;
}

uint8_t Flash_Read_Record_Without_Index (uint32_t andress, sData* str)
{
    uint8_t     i = 0, Length = 0;
    uint8_t		ChecksumByte = 0;
    uint8_t     FirstByte = 0;
    uint8_t     aTEMPData[256] = {0};
    FirstByte = *(__IO uint8_t*)(andress);
    Length = *(__IO uint8_t*)(andress + 1);
    if ((Length >= 255) ||(FirstByte != 0xAA))
    {
        return 0;
    }
    //Doc ra data
    OnchipFlashReadData(andress + 2, &aTEMPData[0], Length);
    //check crc
	for(i = 0; i < (Length-1); i++)
		ChecksumByte ^= aTEMPData[i];
    if (ChecksumByte == aTEMPData[Length-1])
    {
        for(i = 0; i < Length; i++)
            *(str->Data_a8 + str->Length_u16++) = aTEMPData[i];
        return 1;
    }
    return 0;
}

/**
 * @brief	Read last Record: TSVH, Event...
 */
uint8_t Flash_Read_Last_Record (StructManageRecordFlash sRecord, uint32_t *LastPulse, ST_TIME_FORMAT *LastSTime)
{
    uint32_t        Pulse = 0;
    uint8_t         aTemp[256] = {0};
    sData           strTemp = {&aTemp[0], 0};
    uint16_t		IndexRead = 0;
    if (sRecord.IndexSave_u16 < 1)                                //Vi tri save la 0 thi doc truoc do 1 index moi co data
        IndexRead = sRecord.IndexSave_u16 + sRecord.MaxRecord_u16 - 1;
    else
        IndexRead = sRecord.IndexSave_u16 - 1;
    //Doc lai record cuoi cung de lay thoi gian + so xung.
    if (Flash_Read_Record(IndexRead * sRecord.SizeRecord_u16 + sRecord.AddStart_u32, &strTemp, 1) != 0)   //Co ban ghi cuoi cung
    {
        //Thoi gian tu byte 0 den byte 7 bao gom ca obis va length
        LastSTime->year  = aTemp[4];
        LastSTime->month = aTemp[5];
        LastSTime->date  = aTemp[6];
        LastSTime->hour  = aTemp[7];
        LastSTime->min   = aTemp[8];
        LastSTime->sec   = aTemp[9];
        //Pulse tu byte thu 8 den 13 ke ca obis va length
        Pulse = ((aTemp[12] << 24) | (aTemp[13] << 16) |(aTemp[14] << 8) | aTemp[15]);
        *LastPulse = Pulse;
        return 1;
    }
    return 0;
}

/**
 * @brief	Save Data to Flash
 */
uint8_t Flash_Save_Record (StructManageRecordFlash *sRecord, uint8_t *pData, uint8_t Length)
{
    uint32_t AddFlashWrite_u32 = 0;
    uint8_t Result = FALSE;
    //Luu vao Flash
    //Check xem co phai start flash k? se xoa sector tiep theo
    AddFlashWrite_u32 = sRecord->AddStart_u32 + sRecord->IndexSave_u16 * sRecord->SizeRecord_u16;

    if (_ON_FLASH_IS_NEW_PAGE (AddFlashWrite_u32) == 0)
        Erase_Firmware(AddFlashWrite_u32, 1);

    Result = Save_Array_without_erase (AddFlashWrite_u32, pData, Length);  //cho du luu vao thanh cong hay khong cung phai tang index

    //kiem tra xem ban ghi moi nay da vuot qua max chua.
    sRecord->IndexSave_u16++;
    sRecord->IndexSave_u16 = sRecord->IndexSave_u16 % sRecord->MaxRecord_u16;
    //luu lai vi tri Save
    Flash_Save_Index(sRecord->AddIndexSave_u32, sRecord->IndexSave_u16);
    //kiem tra xem ban ghi moi nay da vuot qua Indexsend. thi cung day 2 th len 1
    if (sRecord->IndexSave_u16 == sRecord->IndexSend_u16)
    {
        sRecord->IndexSend_u16 = (sRecord->IndexSend_u16 + 1) % sRecord->MaxRecord_u16;
        //luu IndexSend vao Flash
        Flash_Save_Index(sRecord->AddIndexSend_u32, sRecord->IndexSend_u16);
    }
    return Result;
}

/**
 * @brief	Save Index
 * @param	Index Send: type 0
 * 			Index Save: type 1
 * @retval
 */
void Flash_Save_Index (uint32_t Add, uint16_t Value)
{
    Flash_Save_U16 (Add, Value);
}

/* Private functions definition -----------------------------------------------*/
void Flash_Init_Record_Index (StructManageRecordFlash *sRecord)
{
    uint8_t temp = 0;
    uint8_t Buff_temp[20] = {0};
    uint16_t Length = 0;
    //Doc vi tri send va luu ra
    temp = *(__IO uint8_t*) sRecord->AddIndexSend_u32;
	if (temp != FLASH_BYTE_EMPTY)
	{
        Flash_Get_Infor (sRecord->AddIndexSend_u32 + 1, &Buff_temp[0], &Length, 2);
        sRecord->IndexSend_u16 = (Buff_temp[0] << 8) | Buff_temp[1];
        Flash_Get_Infor (sRecord->AddIndexSave_u32 + 1, &Buff_temp[0], &Length, 2);
        sRecord->IndexSave_u16 = (Buff_temp[0] << 8) | Buff_temp[1];
        //kiem tra dieu kien gioi han InDex
        if (sRecord->IndexSend_u16 >= sRecord->MaxRecord_u16)
        	sRecord->IndexSend_u16 = 0;
        if (sRecord->IndexSave_u16 >= sRecord->MaxRecord_u16)
        	sRecord->IndexSave_u16 = 0;
    } else
    {
    	Flash_Save_Index(sRecord->AddIndexSend_u32, sRecord->IndexSend_u16);
    	Flash_Save_Index(sRecord->AddIndexSave_u32, sRecord->IndexSave_u16);
    }
}

/**
 * @brief	Init Infor from Flash
 */
void Flash_Get_Infor (uint32_t AddStart, uint8_t *pData, uint16_t *Length, uint8_t MAX_LEGNTH_INFOR)
{
    uint8_t LenTemp = 0;

    LenTemp = *(__IO uint8_t*) (AddStart);

    if (LenTemp > MAX_LEGNTH_INFOR)
        LenTemp = MAX_LEGNTH_INFOR;
    //clear buff
    UTIL_MEM_set (pData, 0, MAX_LEGNTH_INFOR);
    //Read flash
    OnchipFlashReadData ((AddStart + 1), pData, LenTemp);
    *Length = LenTemp;
}

/* End of file ----------------------------------------------------------------*/
