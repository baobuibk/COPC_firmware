/*
 * watchdog.c
 *
 *  Created on: May 24, 2024
 *      Author: CAO HIEU
 */


#include "watchdog.h"

/* Private define ------------------------------------------------------------*/
#define	HIGH_PERIOD			200
#define	LOW_PERIOD			1000

typedef struct WDT_TaskContextTypedef
{
	SCH_TASK_HANDLE               taskHandle;
	SCH_TaskPropertyTypedef       taskProperty;
} WDT_TaskContextTypedef;

static uint8_t WDT_Current = 0;

static WDT_TaskContextTypedef           wdt_task_context =
{
	SCH_INVALID_TASK_HANDLE,                 // Will be updated by Schedular
	{
		SCH_TASK_SYNC,                      // taskType;
		SCH_TASK_PRIO_0,                    // taskPriority;
		100,                                // taskPeriodInMS;
		status_wdt_update					// taskFunction;
	}
};




void WDT_init(void)
{
	WDT_Current = 0;

}

void WDT_create_task(void)
{
	SCH_TASK_CreateTask(&wdt_task_context.taskHandle, &wdt_task_context.taskProperty);

}

void	status_wdt_update(void)
{

		if (WDT_Current){
			if (SCH_TIM_HasCompleted(SCH_TIM_WDT))
			{
				WDT_Current = 0;
				LL_GPIO_ResetOutputPin(WD_DONE_GPIO_Port, WD_DONE_Pin);
				SCH_TIM_Start(SCH_TIM_WDT,LOW_PERIOD);	//restart
			}
		} else {
			if (SCH_TIM_HasCompleted(SCH_TIM_WDT))
			{
				WDT_Current = 1;
				LL_GPIO_SetOutputPin(WD_DONE_GPIO_Port, WD_DONE_Pin);
				SCH_TIM_Start(SCH_TIM_WDT,HIGH_PERIOD);	//restart
			}
	}
}
