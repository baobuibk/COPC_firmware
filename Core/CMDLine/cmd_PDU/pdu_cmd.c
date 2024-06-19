/*
 * pdu_cmd.c
 *
 *  Created on: Jun 10, 2024
 *      Author: CAO HIEU
 */

#include "../../ThirdParty/libfsp/fsp.h"
#include "pdu_cmd.h"
#include "../cmdline.h"
#include <stdlib.h>
#include "../../BSP/UART/uart.h"
#include "../../Scheduler/scheduler.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_it.h"
#include "main.h"
#include "../ACK_packet/ACKsend_packet.h"
#include "../global_vars.h"
#include "../rs422.h"
#define PDU_PERIOD 1000

static void PDU_update_task(void);

uint8_t pdu_frame[] = {0xCA, 0x01, 0x03, 0x01, 0x04, 0x06, 0xAA, 0xBF, 0xEF};
typedef struct PDU_TaskContextTypedef
{
	SCH_TASK_HANDLE               taskHandle;
	SCH_TaskPropertyTypedef       taskProperty;
} PDU_TaskContextTypedef;


static PDU_TaskContextTypedef           PDU_task_context =
{
	SCH_INVALID_TASK_HANDLE,                 // Will be updated by Schedular
	{
		SCH_TASK_SYNC,                      // taskType;
		SCH_TASK_PRIO_0,                    // taskPriority;
		100,                                // taskPeriodInMS;
		PDU_update_task					// taskFunction;
	}
};



void PDU_create_task(void)
{
    SCH_TASK_CreateTask(&PDU_task_context.taskHandle, &PDU_task_context.taskProperty);
    SCH_TIM_Start(SCH_TIM_PDU, PDU_PERIOD);
    Ringbuf_init();
}


volatile uint8_t timeout_counter_pdu = 0;

void PDU_update_task(void) {
	if (auto_report_enabled) {

//	if  not in send and wait

		uint8_t *frame;
		uint8_t frame_len;
		if (SCH_TIM_HasCompleted(SCH_TIM_PDU))
		{

			if(!sendFlag){
				if(!send_rs422){
					if(receive_iouFlag&&receive_pmuFlag){
						switch_board(0);
						Uart_flush(USART1);

						frame = pdu_frame;
						frame_len = sizeof(pdu_frame);
						for (int i = 0; i < frame_len; i++) {
							Uart_write(USART1, frame[i]);
						}
						receive_pduFlag = 0;
						send_rs422 = 1;
						SCH_TIM_Start(SCH_TIM_PDU, PDU_PERIOD);
					}
					if(!receive_pduFlag){
						timeout_counter_pdu++;
						if (timeout_counter_pdu > 2){
							disconnect_counter_pdu++;
							timeout_counter_pdu = 0;
							receive_pduFlag = 1;
							if(disconnect_counter_pdu> 4){
								for (int i = 1; i <= 54; i++) {
									    sourceArray[i + 42] = 0xFF; //43   pay1    + 44  pay2        96-<54
									}
							}
						}
					}
				}
			}
		}
	}
}


void	pdu_create_task(void)
{
	SCH_TASK_CreateTask(&PDU_task_context.taskHandle, &PDU_task_context.taskProperty);
	Ringbuf_init();
}



#define DEST_ADDR FSP_ADR_PDU
//volatile uint8_t uart_choose_uart5 = 0;
int Cmd_pdu_set_channel(int argc, char *argv[])
{
    if ((argc-1) < 3) return CMDLINE_TOO_FEW_ARGS;
    if ((argc-1) > 3) return CMDLINE_TOO_MANY_ARGS;
    uint8_t channel = atoi(argv[1]);
    if (channel > 9)   return CMDLINE_INVALID_ARG;

    uint8_t state = atoi(argv[2]);
    if (state > 1) return CMDLINE_INVALID_ARG;

    // BA
    /*
:  --> 00   -> PDU (*)
:  --> 01   -> PMU
:  --> 10   -> CAM
:  --> 11   -> IOU
     */
    LL_GPIO_ResetOutputPin(GPIOA, BOARD_SEL_B_Pin);
    LL_GPIO_ResetOutputPin(GPIOA, BOARD_SEL_A_Pin);
    USART_TypeDef* USARTx = (USART_TypeDef*)argv[argc-1];
    if (USARTx == UART5) {
    	uart_choose_uart5 = 1;
    }else{
    	uart_choose_uart5 = 0;
    }
    uint8_t cmd  = CMD_CODE_PDU_SET_CHANNEL;
    uint8_t payload[2];
    payload[0]  = channel;
    payload[1]  = state;


    fsp_packet_t fsp_pkt;
    fsp_gen_cmd_w_data_pkt(cmd, payload, sizeof(payload), DEST_ADDR, FSP_PKT_WITH_ACK, &fsp_pkt);

    uint8_t encoded_frame[FSP_PKT_MAX_LENGTH];
    uint8_t frame_len;

    frame_encode(&fsp_pkt, encoded_frame, &frame_len);

 //   SCH_Delay(5);
    if (frame_len > 0) {
        for (int i = 0; i < frame_len; i++) {
            Uart_write(USART1, encoded_frame[i]);
        }
        set_fsp_packet(encoded_frame, frame_len);
        set_send_flag();
    }else{
        fsp_gen_cmd_w_data_pkt(cmd, payload, sizeof(payload), DEST_ADDR, FSP_PKT_WITH_ACK, &fsp_pkt);
        frame_encode(&fsp_pkt, encoded_frame, &frame_len);
		set_fsp_packet(encoded_frame, frame_len);
		set_send_flag();
    }

    return CMDLINE_PENDING;
}

int Cmd_pdu_set_buck(int argc, char *argv[])
{
    if ((argc-1) < 3) return CMDLINE_TOO_FEW_ARGS;
    if ((argc-1) > 3) return CMDLINE_TOO_MANY_ARGS;
    uint8_t buck = atoi(argv[1]);
    if (buck > 6)   return CMDLINE_INVALID_ARG;

    uint8_t state = atoi(argv[2]);
    if (state > 1) return CMDLINE_INVALID_ARG;

    // BA
    /*
:  --> 00   -> PDU (*)
:  --> 01   -> PMU
:  --> 10   -> CAM
:  --> 11   -> IOU
     */
    LL_GPIO_ResetOutputPin(GPIOA, BOARD_SEL_B_Pin);
    LL_GPIO_ResetOutputPin(GPIOA, BOARD_SEL_A_Pin);
    USART_TypeDef* USARTx = (USART_TypeDef*)argv[argc-1];
    if (USARTx == UART5) {
    	uart_choose_uart5 = 1;
    }else{
    	uart_choose_uart5 = 0;
    }
    uint8_t cmd  = CMD_CODE_PDU_SET_BUCK;
    uint8_t payload[2];
    payload[0]  = buck;
    payload[1]  = state;

    fsp_packet_t fsp_pkt;
    fsp_gen_cmd_w_data_pkt(cmd, payload, sizeof(payload), DEST_ADDR, FSP_PKT_WITH_ACK, &fsp_pkt);

    uint8_t encoded_frame[FSP_PKT_MAX_LENGTH];
    uint8_t frame_len;

    frame_encode(&fsp_pkt, encoded_frame, &frame_len);

 //   SCH_Delay(5);
    if (frame_len > 0) {
        for (int i = 0; i < frame_len; i++) {
            Uart_write(USART1, encoded_frame[i]);
        }
        set_fsp_packet(encoded_frame, frame_len);
        set_send_flag();
    }else{
        fsp_gen_cmd_w_data_pkt(cmd, payload, sizeof(payload), DEST_ADDR, FSP_PKT_WITH_ACK, &fsp_pkt);
        frame_encode(&fsp_pkt, encoded_frame, &frame_len);
		set_fsp_packet(encoded_frame, frame_len);
		set_send_flag();
    }

    return CMDLINE_PENDING;
}

int Cmd_pdu_set_all(int argc, char *argv[])
{
    if ((argc-1) < 2) return CMDLINE_TOO_FEW_ARGS;
    if ((argc-1) > 2) return CMDLINE_TOO_MANY_ARGS;

    uint8_t state = atoi(argv[1]);
    if (state > 1) return CMDLINE_INVALID_ARG;

    // BA
    /*
:  --> 00   -> PDU (*)
:  --> 01   -> PMU
:  --> 10   -> CAM
:  --> 11   -> IOU
     */
    LL_GPIO_ResetOutputPin(GPIOA, BOARD_SEL_B_Pin);
    LL_GPIO_ResetOutputPin(GPIOA, BOARD_SEL_A_Pin);
    USART_TypeDef* USARTx = (USART_TypeDef*)argv[argc-1];
    if (USARTx == UART5) {
    	uart_choose_uart5 = 1;
    }else{
    	uart_choose_uart5 = 0;
    }
    uint8_t cmd  = CMD_CODE_PDU_SET_ALL;
    uint8_t payload[1];
    payload[0]  = state;

    fsp_packet_t fsp_pkt;
    fsp_gen_cmd_w_data_pkt(cmd, payload, sizeof(payload), DEST_ADDR, FSP_PKT_WITH_ACK, &fsp_pkt);

    uint8_t encoded_frame[FSP_PKT_MAX_LENGTH];
    uint8_t frame_len;

    frame_encode(&fsp_pkt, encoded_frame, &frame_len);

    if (frame_len > 0) {
        for (int i = 0; i < frame_len; i++) {
            Uart_write(USART1, encoded_frame[i]);
        }
        set_fsp_packet(encoded_frame, frame_len);
        set_send_flag();
    }else{
        fsp_gen_cmd_w_data_pkt(cmd, payload, sizeof(payload), DEST_ADDR, FSP_PKT_WITH_ACK, &fsp_pkt);
        frame_encode(&fsp_pkt, encoded_frame, &frame_len);
		set_fsp_packet(encoded_frame, frame_len);
		set_send_flag();
    }


    return CMDLINE_PENDING;
}

int Cmd_pdu_get_channel(int argc, char *argv[])
{
    if ((argc-1) < 2) return CMDLINE_TOO_FEW_ARGS;
    if ((argc-1) > 2) return CMDLINE_TOO_MANY_ARGS;
    uint8_t channel = atoi(argv[1]);
    if (channel > 9)   return CMDLINE_INVALID_ARG;

    // BA
    /*
:  --> 00   -> PDU (*)
:  --> 01   -> PMU
:  --> 10   -> CAM
:  --> 11   -> IOU
     */
    LL_GPIO_ResetOutputPin(GPIOA, BOARD_SEL_B_Pin);
    LL_GPIO_ResetOutputPin(GPIOA, BOARD_SEL_A_Pin);
    USART_TypeDef* USARTx = (USART_TypeDef*)argv[argc-1];
    if (USARTx == UART5) {
    	uart_choose_uart5 = 1;
    }else{
    	uart_choose_uart5 = 0;
    }

    uint8_t cmd  = CMD_CODE_PDU_GET_CHANNEL;
    uint8_t payload[1];
    payload[0]  = channel;

    fsp_packet_t fsp_pkt;
    fsp_gen_cmd_w_data_pkt(cmd, payload, sizeof(payload), DEST_ADDR, FSP_PKT_WITH_ACK, &fsp_pkt);

    uint8_t encoded_frame[FSP_PKT_MAX_LENGTH];
    uint8_t frame_len;

    frame_encode(&fsp_pkt, encoded_frame, &frame_len);

 //   SCH_Delay(5);
    if (frame_len > 0) {
        for (int i = 0; i < frame_len; i++) {
            Uart_write(USART1, encoded_frame[i]);
        }
        set_fsp_packet(encoded_frame, frame_len);
        set_send_flag();
    }else{
        fsp_gen_cmd_w_data_pkt(cmd, payload, sizeof(payload), DEST_ADDR, FSP_PKT_WITH_ACK, &fsp_pkt);
        frame_encode(&fsp_pkt, encoded_frame, &frame_len);
		set_fsp_packet(encoded_frame, frame_len);
		set_send_flag();
    }

    return CMDLINE_PENDING;
}

int Cmd_pdu_get_buck(int argc, char *argv[])
{
    if ((argc-1) < 2) return CMDLINE_TOO_FEW_ARGS;
    if ((argc-1) > 2) return CMDLINE_TOO_MANY_ARGS;
    uint8_t buck = atoi(argv[1]);
    if (buck > 6)   return CMDLINE_INVALID_ARG;

    // BA
    /*
:  --> 00   -> PDU (*)
:  --> 01   -> PMU
:  --> 10   -> CAM
:  --> 11   -> IOU
     */
    LL_GPIO_ResetOutputPin(GPIOA, BOARD_SEL_B_Pin);
    LL_GPIO_ResetOutputPin(GPIOA, BOARD_SEL_A_Pin);
    USART_TypeDef* USARTx = (USART_TypeDef*)argv[argc-1];
    if (USARTx == UART5) {
    	uart_choose_uart5 = 1;
    }else{
    	uart_choose_uart5 = 0;
    }
    uint8_t cmd  = CMD_CODE_PDU_GET_BUCK;
    uint8_t payload[1];
    payload[0]  = buck;

    fsp_packet_t fsp_pkt;
    fsp_gen_cmd_w_data_pkt(cmd, payload, sizeof(payload), DEST_ADDR, FSP_PKT_WITH_ACK, &fsp_pkt);

    uint8_t encoded_frame[FSP_PKT_MAX_LENGTH];
    uint8_t frame_len;

    frame_encode(&fsp_pkt, encoded_frame, &frame_len);

 //   SCH_Delay(5);
    if (frame_len > 0) {
        for (int i = 0; i < frame_len; i++) {
            Uart_write(USART1, encoded_frame[i]);
        }
        set_fsp_packet(encoded_frame, frame_len);
        set_send_flag();
    }else{
        fsp_gen_cmd_w_data_pkt(cmd, payload, sizeof(payload), DEST_ADDR, FSP_PKT_WITH_ACK, &fsp_pkt);
        frame_encode(&fsp_pkt, encoded_frame, &frame_len);
		set_fsp_packet(encoded_frame, frame_len);
		set_send_flag();
    }


    return CMDLINE_PENDING;
}

int Cmd_pdu_get_all(int argc, char *argv[])
{
    if ((argc-1) < 1) return CMDLINE_TOO_FEW_ARGS;
    if ((argc-1) > 1) return CMDLINE_TOO_MANY_ARGS;

    // BA
    /*
:  --> 00   -> PDU (*)
:  --> 01   -> PMU
:  --> 10   -> CAM
:  --> 11   -> IOU
     */
    LL_GPIO_ResetOutputPin(GPIOA, BOARD_SEL_B_Pin);
    LL_GPIO_ResetOutputPin(GPIOA, BOARD_SEL_A_Pin);
    USART_TypeDef* USARTx = (USART_TypeDef*)argv[argc-1];
    if (USARTx == UART5) {
    	uart_choose_uart5 = 1;
    }else{
    	uart_choose_uart5 = 0;
    }
    uint8_t cmd  = CMD_CODE_PDU_GET_ALL;


    fsp_packet_t fsp_pkt;
    fsp_gen_cmd_pkt(cmd, DEST_ADDR, FSP_PKT_WITH_ACK, &fsp_pkt);

    uint8_t encoded_frame[FSP_PKT_MAX_LENGTH];
    uint8_t frame_len;

    frame_encode(&fsp_pkt, encoded_frame, &frame_len);

 //   SCH_Delay(5);
    if (frame_len > 0) {
        for (int i = 0; i < frame_len; i++) {
            Uart_write(USART1, encoded_frame[i]);
        }
        set_fsp_packet(encoded_frame, frame_len);
        set_send_flag();
    }else{
        fsp_gen_cmd_pkt(cmd, DEST_ADDR, FSP_PKT_WITH_ACK, &fsp_pkt);
        frame_encode(&fsp_pkt, encoded_frame, &frame_len);
		set_fsp_packet(encoded_frame, frame_len);
		set_send_flag();
    }

    return CMDLINE_PENDING;
}
