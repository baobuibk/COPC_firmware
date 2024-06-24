/*
 * gps.c
 *
 *  Created on: Jun 20, 2024
 *      Author: nnmin
 */

#include "gps.h"
#include "../../Scheduler/scheduler.h"
#include <stm32f4xx_ll_usart.h>
#include "../../Core/CMDLine/global_vars.h"
#include <stdio.h>
#include "../../BSP/UART/uart.h"

/*Private define*/
static void GPS_task_update(void);

/*Private typedef*/
typedef struct GPS_TaskContextTypedef
{
	SCH_TASK_HANDLE               taskHandle;
	SCH_TaskPropertyTypedef       taskProperty;
}GPS_TaskContextTypedef;

static GPS_TaskContextTypedef		gps_task_context =
{
		SCH_INVALID_TASK_HANDLE,                 // Will be updated by Schedular
		{
			SCH_TASK_SYNC,                      // taskType;
			SCH_TASK_PRIO_0,                    // taskPriority;
			1000,                               // taskPeriodInMS;
			GPS_task_update					// taskFunction;
		}
};


void GPS_create_task(void)
{
	Ringbuf_init();
	SCH_TASK_CreateTask(&gps_task_context.taskHandle, &gps_task_context.taskProperty);
}



static void GPS_task_update(void)
{
	if(gps_report_enable){
		uint8_t rxData;
		while (IsDataAvailable(USART3))
		{
			rxData = Uart_read(USART3);
			Uart_write(USART2, rxData);
			Uart_write(USART6, rxData);
		}
	}
}
