/*
 * CPOC.c
 *
 *  Created on: May 25, 2024
 *      Author: CAO HIEU
 */


#include "../../Scheduler/scheduler.h"
#include "CPOC.h"
#include "../../ThirdParty/libfsp/fsp.h"
#include <stdio.h>
#include "../CMDLine/global_vars.h"
#include "../CMDLine/rs422.h"

const char *decode_error_msgs[7] = {
    "Packet ready",
    "Packet not ready",
    "Packet invalid",
    "Packet wrong address",
    "Packet error",
    "Packet CRC failed",
    "Packet with wrong length"
};

//ring_buffer rx_buffer_cpoc = { { 0 }, 0, 0};
//ring_buffer tx_buffer_cpoc = { { 0 }, 0, 0};
//
//ring_buffer *_rx_buffer_cpoc;
//ring_buffer *_tx_buffer_cpoc;

static void COPC_task_update(void);
USART_TypeDef *board_uart = USART1;
#define TIMEOUT_DEF 200  // 1000ms timeout debug ->>>> 20000
uint16_t cpoc_timeout;
static	fsp_packet_t	s_COPC_FspPacket;
static  COPC_Sfp_Payload_t	*s_pCOPC_Sfp_Payload;

typedef struct COPC_TaskContextTypedef
{
	SCH_TASK_HANDLE               taskHandle;
	SCH_TaskPropertyTypedef       taskProperty;
} COPC_TaskContextTypedef;


static COPC_TaskContextTypedef           s_COPC_task_context =
{
	SCH_INVALID_TASK_HANDLE,                 // Will be updated by Schedular
	{
		SCH_TASK_SYNC,                      // taskType;
		SCH_TASK_PRIO_0,                    // taskPriority;
		10,                                // taskPeriodInMS, call the task to check buffer every 10ms
										//with baudrate of 9600, buffer size can be less than 10 bytes
		COPC_task_update                // taskFunction;
	}
};

void	COPC_init(void)
{
	Ringbuf_init();
	fsp_init(FSP_ADR_CPOC);
	s_pCOPC_Sfp_Payload = (COPC_Sfp_Payload_t *)(&s_COPC_FspPacket.payload);
}


volatile uint8_t receiving = 0;
volatile uint8_t receive_buffer[FSP_PKT_MAX_LENGTH];
volatile uint8_t receive_index = 0;

static void COPC_task_update(void)
{
	uint8_t rxData;


    while (IsDataAvailable(USART1))
    {
        rxData = Uart_read(USART1);

//		char pos_str2[10];
//		sprintf(pos_str2, "%d", rxData);
//		Uart_sendstring(UART5, pos_str2);

        if (!receiving) {
            if (rxData == FSP_PKT_SOD) {
                receiving = 1;
                receive_index = 0;
            }
        } else {

            if (rxData == FSP_PKT_EOF) {
                receiving = 0;
                fsp_packet_t fsp_pkt;
                if(send_rs422){
					frame_decode_rs422((uint8_t *)receive_buffer, receive_index, &fsp_pkt);
					frame_processing_rs422(&fsp_pkt);
					receive_pduFlag = 1;
					receive_pmuFlag = 1;
					receive_iouFlag = 1;
					send_rs422 = 0;
                }else{
                    int ret = frame_decode((uint8_t *)receive_buffer, receive_index, &fsp_pkt);

                    if (ret > 0) {
                        char error_msg[50];
                        sprintf(error_msg, "Error: %s\r\n", decode_error_msgs[ret]);
                        Uart_sendstring(UART5, error_msg);
                        Uart_sendstring(USART6, error_msg);
                    }
                }



            }else{
            	receive_buffer[receive_index++] = rxData;
            }

            if (receive_index >= FSP_PKT_MAX_LENGTH) {
                // Frame quá dài, reset lại

                receiving = 0;
            }
    }
}

}
void	COPC_create_task(void)
{
	COPC_init();
	SCH_TASK_CreateTask(&s_COPC_task_context.taskHandle, &s_COPC_task_context.taskProperty);
}
