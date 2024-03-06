#include "user_link.h"

/* Private macro -----------------------------------------------------------*/
/**
 * @brief macro definition to initialize a critical section.
 *
 */
#ifndef UTIL_TIMER_INIT_CRITICAL_SECTION
	#define UTIL_TIMER_INIT_CRITICAL_SECTION( )
#endif

/**
 * @brief macro definition to enter a critical section.
 *
 */
#ifndef UTIL_TIMER_ENTER_CRITICAL_SECTION
	#define UTIL_TIMER_ENTER_CRITICAL_SECTION( )     //UTILS_ENTER_CRITICAL_SECTION( )
#endif

/**
 * @brief macro definition to exit a critical section.
 *
 */
#ifndef UTIL_TIMER_EXIT_CRITICAL_SECTION
	#define UTIL_TIMER_EXIT_CRITICAL_SECTION( )    	// UTILS_EXIT_CRITICAL_SECTION( )
#endif

/* Private variables ---------------------------------------------------------*/
static UTIL_TIMER_Object_t *TimerListHead = NULL;

/* Private functions definition -----------------------------------------------*/
UTIL_TIMER_Status_t UTIL_TIMER_Init(void)
{
	UTIL_TIMER_INIT_CRITICAL_SECTION();
	TimerListHead = NULL;
	return UTIL_TimerDriver.InitTimer();
}

UTIL_TIMER_Status_t UTIL_TIMER_DeInit(void)
{
	return UTIL_TimerDriver.DeInitTimer();
}

UTIL_TIMER_Status_t UTIL_TIMER_Create(UTIL_TIMER_Object_t *TimerObject, uint32_t PeriodValue,
		UTIL_TIMER_Mode_t Mode, void (*Callback)(void*), void *Argument)
{
	if ((TimerObject != NULL) && (Callback != NULL))
	{
		TimerObject->Timestamp = 0U;
		TimerObject->ReloadValue = UTIL_TimerDriver.ms2Tick(PeriodValue);
		TimerObject->IsPending = 0U;
		TimerObject->IsRunning = 0U;
		TimerObject->IsReloadStopped = 0U;     //Danh dau stop timer set period
		TimerObject->Callback = Callback;      //Ham callback
		TimerObject->argument = Argument;      //Doi so cho ham callback
		TimerObject->Mode = Mode;
		TimerObject->Next = NULL;
		return UTIL_TIMER_OK;
	} else
	{
		return UTIL_TIMER_INVALID_PARAM;
	}
}

/**
 * @brief	Start 1 object timer
 * @note	Nếu như timer đó đã tồn tại trong list timer chạy thì sẽ bị bỏ qua.
 */
UTIL_TIMER_Status_t UTIL_TIMER_Start(UTIL_TIMER_Object_t *TimerObject)
{
	UTIL_TIMER_Status_t ret = UTIL_TIMER_OK;
	uint32_t elapsedTime;
	uint32_t minValue;
	uint32_t ticks;

	//Check xem đã tồn tại object trong chuỗi timer được set trước đó chưa. && Timer đó ruuning == 0
	if ((TimerObject != NULL) && (TimerExists(TimerObject) == false)
			&& (TimerObject->IsRunning == 0U))
	{
		UTIL_TIMER_ENTER_CRITICAL_SECTION();
		ticks = TimerObject->ReloadValue;
		minValue = UTIL_TimerDriver.GetMinimumTimeout(); //tra  ve 3 tick

		if (ticks < minValue)
		{
			ticks = minValue;
		}

		TimerObject->Timestamp = ticks;
		TimerObject->IsPending = 0U;
		TimerObject->IsRunning = 1U;
		TimerObject->IsReloadStopped = 0U;
		if (TimerListHead == NULL)           //Neu dang k co alarm nào?
		{
			UTIL_TimerDriver.SetTimerContext();       //Lay RTC va luu gia tri RTC luc hen gio
			TimerInsertNewHeadTimer(TimerObject); /* insert a timeout at now+obj->Timestamp */
		} else
		{
			elapsedTime = UTIL_TimerDriver.GetTimerElapsedTime(); //tinh xem da set alarm dc bao nhieu ms rôi
			TimerObject->Timestamp += elapsedTime;

			if (TimerObject->Timestamp < TimerListHead->Timestamp) //neu timer tiep theo co gia tri nho hon cai dang set
			{
				TimerInsertNewHeadTimer(TimerObject);
			} else
			{
				TimerInsertTimer(TimerObject);     //chèn object vao giữa.
			}
		}
		UTIL_TIMER_EXIT_CRITICAL_SECTION();
	} else
	{
		ret = UTIL_TIMER_INVALID_PARAM;
	}
	return ret;
}

/**
 * @brief	Start timer kem them option set period cho object đó
 */
UTIL_TIMER_Status_t UTIL_TIMER_StartWithPeriod(UTIL_TIMER_Object_t *TimerObject,
		uint32_t PeriodValue)
{
	UTIL_TIMER_Status_t ret = UTIL_TIMER_OK;
	if (NULL == TimerObject)
	{
		ret = UTIL_TIMER_INVALID_PARAM;
	} else
	{
		TimerObject->ReloadValue = UTIL_TimerDriver.ms2Tick(PeriodValue);
		if (TimerExists(TimerObject))
		{
			(void) UTIL_TIMER_Stop(TimerObject);
		}
		ret = UTIL_TIMER_Start(TimerObject);
	}
	return ret;
}


UTIL_TIMER_Status_t UTIL_TIMER_Stop(UTIL_TIMER_Object_t *TimerObject)
{
	UTIL_TIMER_Status_t ret = UTIL_TIMER_OK;

	if (NULL != TimerObject)
	{
		UTIL_TIMER_ENTER_CRITICAL_SECTION();
		UTIL_TIMER_Object_t *prev = TimerListHead;
		UTIL_TIMER_Object_t *cur = TimerListHead;
		TimerObject->IsReloadStopped = 1U;

		/* List is empty or the Obj to stop does not exist  */
		if (NULL != TimerListHead)
		{
			TimerObject->IsRunning = 0U;

			if (TimerListHead == TimerObject) /* Stop the Head */
			{
				TimerListHead->IsPending = 0;
				if (TimerListHead->Next != NULL)
				{
					TimerListHead = TimerListHead->Next;
					TimerSetTimeout(TimerListHead);
				} else
				{
					UTIL_TimerDriver.StopTimerEvt();
					TimerListHead = NULL;
				}
			} else /* Stop an object within the list */
			{
				while (cur != NULL)
				{
					if (cur == TimerObject)
					{
						if (cur->Next != NULL)
						{
							cur = cur->Next;
							prev->Next = cur;
						} else
						{
							cur = NULL;
							prev->Next = cur;
						}
						break;
					} else
					{
						prev = cur;
						cur = cur->Next;
					}
				}
			}
			ret = UTIL_TIMER_OK;
		}
		UTIL_TIMER_EXIT_CRITICAL_SECTION();
	} else
	{
		ret = UTIL_TIMER_INVALID_PARAM;
	}
	return ret;
}

/**
 * @brief	Set lai period 1 timer dang chay.
 * @note	Neu timer chua dc khoi chay thi se khong set
 */
UTIL_TIMER_Status_t UTIL_TIMER_SetPeriod(UTIL_TIMER_Object_t *TimerObject, uint32_t NewPeriodValue)
{
	UTIL_TIMER_Status_t ret = UTIL_TIMER_OK;
	if (NULL == TimerObject)
	{
		ret = UTIL_TIMER_INVALID_PARAM;
	} else
	{
		TimerObject->ReloadValue = UTIL_TimerDriver.ms2Tick(NewPeriodValue);
		if (TimerExists(TimerObject))
		{
			(void) UTIL_TIMER_Stop(TimerObject);
			ret = UTIL_TIMER_Start(TimerObject);
		}
	}
	return ret;
}

UTIL_TIMER_Status_t UTIL_TIMER_SetReloadMode(UTIL_TIMER_Object_t *TimerObject,
		UTIL_TIMER_Mode_t ReloadMode)
{
	UTIL_TIMER_Status_t ret = UTIL_TIMER_OK;
	if (NULL == TimerObject)
	{
		ret = UTIL_TIMER_INVALID_PARAM;
	} else
	{
		TimerObject->Mode = ReloadMode;
	}
	return ret;
}

UTIL_TIMER_Status_t UTIL_TIMER_GetRemainingTime(UTIL_TIMER_Object_t *TimerObject,
		uint32_t *ElapsedTime)
{
	UTIL_TIMER_Status_t ret = UTIL_TIMER_OK;
	if (TimerExists(TimerObject))
	{
		uint32_t time = UTIL_TimerDriver.GetTimerElapsedTime();
		if (TimerObject->Timestamp < time)
		{
			*ElapsedTime = 0;
		} else
		{
			*ElapsedTime = TimerObject->Timestamp - time;
		}
	} else
	{
		ret = UTIL_TIMER_INVALID_PARAM;
	}
	return ret;
}

uint32_t UTIL_TIMER_IsRunning(UTIL_TIMER_Object_t *TimerObject)
{
	if (TimerObject != NULL)
	{
		return TimerObject->IsRunning;
	} else
	{
		return 0;
	}
}

uint32_t UTIL_TIMER_GetFirstRemainingTime(void)
{
	uint32_t NextTimer = 0xFFFFFFFFU;

	if (TimerListHead != NULL)
	{
		(void) UTIL_TIMER_GetRemainingTime(TimerListHead, &NextTimer);
	}
	return NextTimer;
}

/**
 * @brief	Thuc hien callback cua timer va hen alarm tiep theo:
 * @note	da bu thoi gian sai lech, nen có thể thực hiện ở ngoài hàm main.
 */
void UTIL_TIMER_IRQ_Handler(void)
{
	UTIL_TIMER_Object_t *cur;
	uint32_t old, now, DeltaContext;

	old = UTIL_TimerDriver.GetTimerContext();
	now = UTIL_TimerDriver.SetTimerContext();

	DeltaContext = now - old; /*intentional wrap around */

	/* update timeStamp based upon new Time Reference*/
	/* because delta context should never exceed 2^32*/
	if (TimerListHead != NULL)
	{
		cur = TimerListHead;
		do
		{
			if (cur->Timestamp > DeltaContext)
			{
				cur->Timestamp -= DeltaContext;   //tat ca cac timestamp se tru di gia tri sai lech
			} else
			{
				cur->Timestamp = 0;
			}
			cur = cur->Next;
		} while (cur != NULL);
	}

	/* Execute expired timer and update the list */
	while ((TimerListHead != NULL)
			&& ((TimerListHead->Timestamp == 0U)
					|| (TimerListHead->Timestamp < UTIL_TimerDriver.GetTimerElapsedTime())))
	{
		cur = TimerListHead;
		TimerListHead = TimerListHead->Next;
		cur->IsPending = 0;
		cur->IsRunning = 0;
		cur->Callback(cur->argument);          //thuc hien callback cua timer.
		if ((cur->Mode == UTIL_TIMER_PERIODIC) && (cur->IsReloadStopped == 0U))
		{
			(void) UTIL_TIMER_Start(cur);
		}
	}

	/* start the next TimerListHead if it exists and it is not pending*/
	if ((TimerListHead != NULL) && (TimerListHead->IsPending == 0U))
	{
		TimerSetTimeout(TimerListHead);
	}
}

UTIL_TIMER_Time_t UTIL_TIMER_GetCurrentTime(void)
{
	uint32_t now = UTIL_TimerDriver.GetTimerValue();
	return UTIL_TimerDriver.Tick2ms(now);
}

UTIL_TIMER_Time_t UTIL_TIMER_GetElapsedTime(UTIL_TIMER_Time_t past)
{
	uint32_t nowInTicks = UTIL_TimerDriver.GetTimerValue();
	uint32_t pastInTicks = UTIL_TimerDriver.ms2Tick(past);
	/* intentional wrap around. Works Ok if tick duation below 1ms */
	return UTIL_TimerDriver.Tick2ms(nowInTicks - pastInTicks);
}

/**
 * @brief 	Check if the Object to be added is not already in the list
 * @param 	TimerObject Structure containing the timer object parameters
 * @retval 	1 (the object is already in the list) or 0
 */
bool TimerExists(UTIL_TIMER_Object_t *TimerObject)
{
	UTIL_TIMER_Object_t *cur = TimerListHead;
	while (cur != NULL)
	{
		if (cur == TimerObject)
		{
			return true;
		}
		cur = cur->Next;
	}
	return false;
}

/**
 * @brief Sets a timeout with the duration "timestamp"
 * @param TimerObject Structure containing the timer object parameters
 */
void TimerSetTimeout(UTIL_TIMER_Object_t *TimerObject)
{
	uint32_t minTicks = UTIL_TimerDriver.GetMinimumTimeout();
	TimerObject->IsPending = 1;
	/* In case deadline too soon */
	if (TimerObject->Timestamp < (UTIL_TimerDriver.GetTimerElapsedTime() + minTicks))
	{
		TimerObject->Timestamp = UTIL_TimerDriver.GetTimerElapsedTime() + minTicks;
	}
	UTIL_TimerDriver.StartTimerEvt(TimerObject->Timestamp);
}

/**
 * @brief Adds a timer to the list.
 * @remark The list is automatically sorted. The list head always contains the
 *     			next timer to expire.
 * @param TimerObject Structure containing the timer object parameters
 */
void TimerInsertTimer(UTIL_TIMER_Object_t *TimerObject)
{
	UTIL_TIMER_Object_t *cur = TimerListHead;
	UTIL_TIMER_Object_t *next = TimerListHead->Next;
	while (cur->Next != NULL)
	{
		if (TimerObject->Timestamp > next->Timestamp)
		{
			cur = next;
			next = next->Next;
		} else
		{
			cur->Next = TimerObject;
			TimerObject->Next = next;
			return;
		}
	}
	cur->Next = TimerObject;
	TimerObject->Next = NULL;
}

/**
 * @brief 	Adds or replace the head timer of the list.
 * @param 	TimerObject Structure containing the timer object parameters
 * @remark 	The list is automatically sorted. The list head always contains the
 *         		next timer to expire.
 */
void TimerInsertNewHeadTimer(UTIL_TIMER_Object_t *TimerObject)
{
	UTIL_TIMER_Object_t *cur = TimerListHead;
	if (cur != NULL)
	{
		cur->IsPending = 0;
	}
	TimerObject->Next = cur;
	TimerListHead = TimerObject;
	TimerSetTimeout(TimerListHead);
}

/*
 * @brief	Set RTC
 * 				- Tinh delta Timestamp
 * 				- Cập nhật lại timestamp của cac Timer
 * 				- Set RTC
 * 				- Get TimerContext mơi nhất
 * 				- Set lại timer Alarm
 * */
void UTIL_Set_RTC (ST_TIME_FORMAT sTime)
{
	UTIL_TIMER_Object_t *cur;
	uint32_t old, now, DeltaContext;
	old = UTIL_TimerDriver.GetTimerContext();
	now = UTIL_TimerDriver.SetTimerContext();
	DeltaContext = now - old; /*intentional wrap around */
	/* update timeStamp based upon new Time Reference*/
	/* because delta context should never exceed 2^32*/
	if (TimerListHead != NULL)
	{
		cur = TimerListHead;
		do
		{
			if (cur->Timestamp > DeltaContext)
			{
				cur->Timestamp -= DeltaContext;   //tat ca cac timestamp se tru di gia tri sai lech
			} else
			{
				cur->Timestamp = 0;
			}
			cur = cur->Next;
		} while (cur != NULL);
	}
	/* Execute expired timer and update the list */
	while ((TimerListHead != NULL)
			&& ((TimerListHead->Timestamp == 0U)
					|| (TimerListHead->Timestamp < UTIL_TimerDriver.GetTimerElapsedTime())))
	{
		cur = TimerListHead;
		TimerListHead = TimerListHead->Next;
		cur->IsPending = 0;
		cur->IsRunning = 0;
		cur->Callback(cur->argument);          //thuc hien callback cua timer.
		if ((cur->Mode == UTIL_TIMER_PERIODIC) && (cur->IsReloadStopped == 0U))
		{
			(void) UTIL_TIMER_Start(cur);   //If period == head ->start -> Then restart again in TimerSetTimeout
		}
	}
    // Set RTC new
	Set_RTC(sTime);
    //Get Timer Context new
	UTIL_TimerDriver.SetTimerContext();
	/* start the next TimerListHead if it exists and it is not pending*/  //Phai thay doi luon start lai
	if (TimerListHead != NULL)
	{
		TimerSetTimeout(TimerListHead);
	}
}

/* End of file ----------------------------------------------------------------*/
