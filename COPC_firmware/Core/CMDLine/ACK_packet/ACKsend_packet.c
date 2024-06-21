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
#include "../global_vars.h"
/* Private define ------------------------------------------------------------*/
#define	ACK_TIMEOUT			1000
#define MAX_RETRIES 		2

volatile uint8_t g_encoded_pkt[FSP_PKT_MAX_LENGTH];
volatile uint8_t g_encoded_len;
volatile uint8_t g_pkt_lock = 0;

volatile uint8_t sendFlag = 0;
volatile uint8_t retryCount = 0;

void	status_ACKsend_update(void);

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
	while (g_pkt_lock);
	g_pkt_lock = 1;
	memset((void *)g_encoded_pkt, 0, sizeof(g_encoded_pkt));
    memcpy((void *)g_encoded_pkt, encoded_pkt, encoded_len);
    g_encoded_len = encoded_len;
    g_pkt_lock = 0;
    sendFlag = 1;
    SCH_TIM_Start(SCH_TIM_ACK, ACK_TIMEOUT);
}

void send_packet_create_task(void)
{
    SCH_TASK_CreateTask(&ACKsend_task_context.taskHandle, &ACKsend_task_context.taskProperty);
}


uint8_t sendBuffer[FSP_PKT_MAX_LENGTH];

void	status_ACKsend_update(void)
{

			if (SCH_TIM_HasCompleted(SCH_TIM_ACK))
			{

			    if (sendFlag)
			    {
			    	if(retryCount < MAX_RETRIES){
			    		while (g_pkt_lock);
			    		g_pkt_lock = 1;
			            memcpy(sendBuffer, (const void *)g_encoded_pkt, g_encoded_len);
			            for (int i = 0; i < g_encoded_len; i++) {
			                Uart_write(USART1, sendBuffer[i]);
			            }
						retryCount++;
						sendFlag = 1;
						g_pkt_lock = 0;
			    	}
			    	else {
			    		retryCount = 0;
			    		clear_send_flag();

			    		Uart_sendstring(UART5, "TIMEOUT_NORESPONE");
			    		Uart_sendstring(UART5, "\r\n> ");
			    		Uart_sendstring(USART6, "TIMEOUT_NORESPONE");
			    		Uart_sendstring(USART6, "\r\n> ");
			        }


			    }

				SCH_TIM_Start(SCH_TIM_ACK, ACK_TIMEOUT);	//restart
			}
		   else {

			}


		//	[set trang thai]
}
