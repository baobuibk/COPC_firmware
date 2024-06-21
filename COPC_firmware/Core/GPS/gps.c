/*
 * gps.c
 *
 *  Created on: Jun 20, 2024
 *      Author: nnmin
 */

#include "gps.h"
#include "../../Scheduler/scheduler.h"
#include <stm32f4xx_ll_usart.h>
#include "../../Core/CMDLine/command.h"
#include "main.h"
#include "../../BSP/RTC/ds3231.h"
#include "../../BSP/uartstdio/uartstdio.h"
#include <stdio.h>



/*Private define*/
#define BUFFER_SIZE	56
void status_gps_update(void);

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
			status_gps_update					// taskFunction;
		}
};



void GPS_init(void)
{

}

void GPS_create_task(void)
{
	SCH_TASK_CreateTask(&gps_task_context.taskHandle, &gps_task_context.taskProperty);
}

static void GPS_SendData(USART_TypeDef *USARTx, char *buffer, uint32_t bufferSize)
{
    for (uint8_t i = 0; i < bufferSize; i++)
    {
        // Wait until TXE flag is set
        while (!LL_USART_IsActiveFlag_TXE(USARTx))
        {
        }

        // Transmit data
        LL_USART_TransmitData8(USARTx, buffer[i]);
    }

    // Wait until TC flag is set
    while (!LL_USART_IsActiveFlag_TC(USARTx))
    {
    }
}

void status_gps_update(void)
{
	char gps[BUFFER_SIZE] = {0};
	uint32_t count;

	//Nhận data từ gps
	UARTStdioConfig(USART3, true);

	//Kiểm tra gói tín hiệu
	do
	{
		count = UARTgets(gps, BUFFER_SIZE);
	}
	while(count < 38);

	LL_USART_DisableIT_RXNE(USART3);


	//truyền tín hiệu sang RF
	GPS_SendData(USART2, gps, count);
}



