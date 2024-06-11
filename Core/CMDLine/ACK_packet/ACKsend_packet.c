/*
 * send_packet.c
 *
 *  Created on: May 28, 2024
 *      Author: CAO HIEU
 */


#include "ACKsend_packet.h"
#include "../../ThirdParty/libfsp/fsp.h"
#include <string.h>
#include "../../BSP/UART/uart.h"

/* Private define ------------------------------------------------------------*/
#define	ACK_TIMEOUT			1000
#define MAX_RETRIES 		3

uint8_t g_encoded_pkt[FSP_PKT_MAX_LENGTH];
uint8_t g_encoded_len;

volatile uint8_t sendFlag = 0;
volatile uint8_t retryCount = 0;

typedef struct ACKsend_TaskContextTypedef
{
	SCH_TASK_HANDLE               taskHandle;
	SCH_TaskPropertyTypedef       taskProperty;
} ACKsend_TaskContextTypedef;



static ACKsend_TaskContextTypedef           ACKsend_task_context =
{
	SCH_INVALID_TASK_HANDLE,                 // Will be updated by Schedular
	{
		SCH_TASK_SYNC,                      // taskType;
		SCH_TASK_PRIO_0,                    // taskPriority;
		100,                                // taskPeriodInMS;
		status_ACKsend_update					// taskFunction;
	}
};

void send_packet_init(void)
{
    sendFlag = 0;
    retryCount = 0;
}

void set_send_flag(void)
{
    sendFlag = 1;
}

void clear_send_flag(void)
{
    sendFlag = 0;
}


void set_fsp_packet(uint8_t *encoded_pkt, uint8_t encoded_len)
{
	memset(g_encoded_pkt, 0, sizeof(g_encoded_pkt));
    memcpy(g_encoded_pkt, encoded_pkt, encoded_len);
    g_encoded_len = encoded_len;
}

void send_packet_create_task(void)
{
    SCH_TASK_CreateTask(&ACKsend_task_context.taskHandle, &ACKsend_task_context.taskProperty);
}

void	status_ACKsend_update(void)
{

			if (SCH_TIM_HasCompleted(SCH_TIM_ACK))
			{

			    if (sendFlag)
			    {
			    	if(retryCount < MAX_RETRIES){

						for (int i = 0; i < g_encoded_len; i++) {
							Uart_write(USART1, g_encoded_pkt[i]);
						}
						retryCount++;
			    	}
			    	else {
			    		retryCount = 0;
			    		clear_send_flag();
			    		Uart_sendstring(USART6, "TIMEOUT_NORESPONE");
			    		Uart_sendstring(USART6, "\r\n> ");
			        }


			    }

				SCH_TIM_Start(SCH_TIM_ACK,ACK_TIMEOUT);	//restart
			}
		   else {

			}


		//	[set trang thai]
}
