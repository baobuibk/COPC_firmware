/*
 * pmu_cmd.c
 *
 *  Created on: Jun 10, 2024
 *      Author: CAO HIEU
 */
#include "../../ThirdParty/libfsp/fsp.h"
#include "pmu_cmd.h"
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
#define PMU_PERIOD 1000
uint8_t pmu_frame[] = {0xCA, 0x01, 0x02, 0x01, 0x04, 0x08, 0x3D, 0xC5, 0xEF};
static void PMU_update_task(void);


typedef struct PMU_TaskContextTypedef
{
	SCH_TASK_HANDLE               taskHandle;
	SCH_TaskPropertyTypedef       taskProperty;
} PMU_TaskContextTypedef;


static PMU_TaskContextTypedef           PMU_task_context =
{
	SCH_INVALID_TASK_HANDLE,                 // Will be updated by Schedular
	{
		SCH_TASK_SYNC,                      // taskType;
		SCH_TASK_PRIO_0,                    // taskPriority;
		100,                                // taskPeriodInMS;
		PMU_update_task					// taskFunction;
	}
};



void PMU_create_task(void)
{
    SCH_TASK_CreateTask(&PMU_task_context.taskHandle, &PMU_task_context.taskProperty);
    SCH_TIM_Start(SCH_TIM_PMU, PMU_PERIOD);
    Ringbuf_init();
}




volatile uint8_t timeout_counter_pmu = 0;

void PMU_update_task(void) {
	if (auto_report_enabled) {

//	if  not in send and wait

		uint8_t *frame;
		uint8_t frame_len;
		if (SCH_TIM_HasCompleted(SCH_TIM_PMU))
		{

			if(!sendFlag){
				if(!send_rs422){
					if(receive_iouFlag&&receive_pduFlag){
						switch_board(1);
						Uart_flush(USART1);

						frame = pmu_frame;
						frame_len = sizeof(pmu_frame);
						for (int i = 0; i < frame_len; i++) {
							Uart_write(USART1, frame[i]);
						}
						receive_pmuFlag = 0;
						send_rs422 = 1;
						SCH_TIM_Start(SCH_TIM_PMU, PMU_PERIOD);
					}
					if(!receive_pmuFlag){
						timeout_counter_pmu++;
						if (timeout_counter_pmu > 2){
							disconnect_counter_pmu++;
							timeout_counter_pmu = 0;
							receive_pmuFlag = 1;
							if(disconnect_counter_pmu> 4){
								for (int i = 1; i <= 24; i++) {
									sourceArray[i + 96] = 0xFF; //97   pay1    + 98 pay2    120    pay24
								}
							}
						}
					}
				}
			}
		}
	}
}


void	pmu_create_task(void)
{
	SCH_TASK_CreateTask(&PMU_task_context.taskHandle, &PMU_task_context.taskProperty);
	Ringbuf_init();
}


#define DEST_ADDR FSP_ADR_PMU
//volatile uint8_t uart_choose_uart5 = 0;
int Cmd_pmu_get_temp(int argc, char *argv[])
{
    if ((argc-1) < 1) return CMDLINE_TOO_FEW_ARGS;
    if ((argc-1) > 1) return CMDLINE_TOO_MANY_ARGS;

    // BA
    /*
:  --> 00   -> PDU
:  --> 01   -> PMU (*)
:  --> 10   -> CAM
:  --> 11   -> IOU
     */
    LL_GPIO_ResetOutputPin(GPIOA, BOARD_SEL_B_Pin);
    LL_GPIO_SetOutputPin(GPIOA, BOARD_SEL_A_Pin);
    USART_TypeDef* USARTx = (USART_TypeDef*)argv[argc-1];
    if (USARTx == UART5) {
    	uart_choose_uart5 = 1;
    }else{
    	uart_choose_uart5 = 0;
    }
    uint8_t cmd  = CMD_CODE_PMU_GET_TEMP;
    fsp_packet_t fsp_pkt;

    fsp_gen_cmd_pkt(cmd, DEST_ADDR, FSP_PKT_WITH_ACK, &fsp_pkt);

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
        fsp_gen_cmd_pkt(cmd, DEST_ADDR, FSP_PKT_WITH_ACK, &fsp_pkt);
        frame_encode(&fsp_pkt, encoded_frame, &frame_len);
		set_fsp_packet(encoded_frame, frame_len);
		set_send_flag();
    }

    return CMDLINE_PENDING;
}

int Cmd_pmu_bat_vol(int argc, char *argv[])
{
    if ((argc-1) < 1) return CMDLINE_TOO_FEW_ARGS;
    if ((argc-1) > 1) return CMDLINE_TOO_MANY_ARGS;
    // BA
    /*
:  --> 00   -> PDU
:  --> 01   -> PMU (*)
:  --> 10   -> CAM
:  --> 11   -> IOU
     */
    LL_GPIO_ResetOutputPin(GPIOA, BOARD_SEL_B_Pin);
    LL_GPIO_SetOutputPin(GPIOA, BOARD_SEL_A_Pin);
    USART_TypeDef* USARTx = (USART_TypeDef*)argv[argc-1];
    if (USARTx == UART5) {
    	uart_choose_uart5 = 1;
    }else{
    	uart_choose_uart5 = 0;
    }
    uint8_t cmd  = CMD_CODE_BAT_VOL;
    fsp_packet_t fsp_pkt;

    fsp_gen_cmd_pkt(cmd, DEST_ADDR, FSP_PKT_WITH_ACK, &fsp_pkt);

    uint8_t encoded_frame[FSP_PKT_MAX_LENGTH];
    uint8_t frame_len;

    frame_encode(&fsp_pkt, encoded_frame, &frame_len);

  //  SCH_Delay(5);
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

int Cmd_pmu_parag_in(int argc, char *argv[])
{
    if ((argc-1) < 1) return CMDLINE_TOO_FEW_ARGS;
    if ((argc-1) > 1) return CMDLINE_TOO_MANY_ARGS;

    // BA
    /*
:  --> 00   -> PDU
:  --> 01   -> PMU (*)
:  --> 10   -> CAM
:  --> 11   -> IOU
     */
    LL_GPIO_ResetOutputPin(GPIOA, BOARD_SEL_B_Pin);
    LL_GPIO_SetOutputPin(GPIOA, BOARD_SEL_A_Pin);
    USART_TypeDef* USARTx = (USART_TypeDef*)argv[argc-1];
    if (USARTx == UART5) {
    	uart_choose_uart5 = 1;
    }else{
    	uart_choose_uart5 = 0;
    }
    uint8_t cmd  = CMD_CODE_PARAG_IN;
    fsp_packet_t fsp_pkt;

    fsp_gen_cmd_pkt(cmd, DEST_ADDR, FSP_PKT_WITH_ACK, &fsp_pkt);

    uint8_t encoded_frame[FSP_PKT_MAX_LENGTH];
    uint8_t frame_len;

    frame_encode(&fsp_pkt, encoded_frame, &frame_len);

  //  SCH_Delay(5);
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

int Cmd_pmu_parag_out(int argc, char *argv[])
{
    if ((argc-1) < 1) return CMDLINE_TOO_FEW_ARGS;
    if ((argc-1) > 1) return CMDLINE_TOO_MANY_ARGS;
    // BA
    /*
:  --> 00   -> PDU
:  --> 01   -> PMU (*)
:  --> 10   -> CAM
:  --> 11   -> IOU
     */
    LL_GPIO_ResetOutputPin(GPIOA, BOARD_SEL_B_Pin);
    LL_GPIO_SetOutputPin(GPIOA, BOARD_SEL_A_Pin);
    USART_TypeDef* USARTx = (USART_TypeDef*)argv[argc-1];
    if (USARTx == UART5) {
    	uart_choose_uart5 = 1;
    }else{
    	uart_choose_uart5 = 0;
    }
    uint8_t cmd  = CMD_CODE_PARAG_OUT;
    fsp_packet_t fsp_pkt;

    fsp_gen_cmd_pkt(cmd, DEST_ADDR, FSP_PKT_WITH_ACK, &fsp_pkt);

    uint8_t encoded_frame[FSP_PKT_MAX_LENGTH];
    uint8_t frame_len;

    frame_encode(&fsp_pkt, encoded_frame, &frame_len);

   // SCH_Delay(5);
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

int Cmd_pmu_set_temppoint(int argc, char *argv[])
{
    if ((argc-1) < 3) return CMDLINE_TOO_FEW_ARGS;
    if ((argc-1) > 3) return CMDLINE_TOO_MANY_ARGS;
    uint16_t lowpoint = atoi(argv[1]);
   // if (lowpoint > 6)   return CMDLINE_INVALID_ARG;
    uint16_t highpoint = atoi(argv[2]);
   // if (highpoint > 6)   return CMDLINE_INVALID_ARG;
    if (highpoint < lowpoint)   return CMDLINE_INVALID_ARG;
    // BA
    /*
:  --> 00   -> PDU
:  --> 01   -> PMU (*)
:  --> 10   -> CAM
:  --> 11   -> IOU
     */
    LL_GPIO_ResetOutputPin(GPIOA, BOARD_SEL_B_Pin);
    LL_GPIO_SetOutputPin(GPIOA, BOARD_SEL_A_Pin);
    USART_TypeDef* USARTx = (USART_TypeDef*)argv[argc-1];
    if (USARTx == UART5) {
    	uart_choose_uart5 = 1;
    }else{
    	uart_choose_uart5 = 0;
    }
    uint8_t cmd  = CMD_CODE_PMU_TEMP_POINT;

    uint8_t payload[4];
    payload[0]  = (uint8_t)(lowpoint >> 8);
    payload[1]  = (uint8_t)(lowpoint & 0xFF);
    payload[2]  = (uint8_t)(highpoint >> 8);
    payload[3]  = (uint8_t)(highpoint & 0xFF);

    fsp_packet_t fsp_pkt;
    fsp_gen_cmd_w_data_pkt(cmd, payload, sizeof(payload), DEST_ADDR, FSP_PKT_WITH_ACK, &fsp_pkt);

    uint8_t encoded_frame[FSP_PKT_MAX_LENGTH];
    uint8_t frame_len;

    frame_encode(&fsp_pkt, encoded_frame, &frame_len);

  //  SCH_Delay(5);
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

int Cmd_pmu_set_output(int argc, char *argv[])
{
    if ((argc-1) < 2) return CMDLINE_TOO_FEW_ARGS;
    if ((argc-1) > 2) return CMDLINE_TOO_MANY_ARGS;
    uint8_t state = atoi(argv[1]);
    if (state > 1)   return CMDLINE_INVALID_ARG;
    // BA
    /*
:  --> 00   -> PDU
:  --> 01   -> PMU (*)
:  --> 10   -> CAM
:  --> 11   -> IOU
     */
    LL_GPIO_ResetOutputPin(GPIOA, BOARD_SEL_B_Pin);
    LL_GPIO_SetOutputPin(GPIOA, BOARD_SEL_A_Pin);
    USART_TypeDef* USARTx = (USART_TypeDef*)argv[argc-1];
    if (USARTx == UART5) {
    	uart_choose_uart5 = 1;
    }else{
    	uart_choose_uart5 = 0;
    }

    uint8_t cmd  = CMD_CODE_PMU_OUTPUT;

    uint8_t payload[4];
    payload[0]  = state;


    fsp_packet_t fsp_pkt;
    fsp_gen_cmd_w_data_pkt(cmd, payload, sizeof(payload), DEST_ADDR, FSP_PKT_WITH_ACK, &fsp_pkt);

    uint8_t encoded_frame[FSP_PKT_MAX_LENGTH];
    uint8_t frame_len;

    frame_encode(&fsp_pkt, encoded_frame, &frame_len);

   // SCH_Delay(5);
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

int Cmd_pmu_set_pwm(int argc, char *argv[])
{
    if ((argc-1) < 2) return CMDLINE_TOO_FEW_ARGS;
    if ((argc-1) > 2) return CMDLINE_TOO_MANY_ARGS;
    uint8_t duty = atoi(argv[1]);
    if (duty > 100)   return CMDLINE_INVALID_ARG;
    // BA
    /*
:  --> 00   -> PDU
:  --> 01   -> PMU (*)
:  --> 10   -> CAM
:  --> 11   -> IOU
     */
    LL_GPIO_ResetOutputPin(GPIOA, BOARD_SEL_B_Pin);
    LL_GPIO_SetOutputPin(GPIOA, BOARD_SEL_A_Pin);
    USART_TypeDef* USARTx = (USART_TypeDef*)argv[argc-1];
    if (USARTx == UART5) {
    	uart_choose_uart5 = 1;
    }else{
    	uart_choose_uart5 = 0;
    }
    uint8_t cmd  = CMD_CODE_PMU_PWM;

    uint8_t payload[4];
    payload[0]  = duty;


    fsp_packet_t fsp_pkt;
    fsp_gen_cmd_w_data_pkt(cmd, payload, sizeof(payload), DEST_ADDR, FSP_PKT_WITH_ACK, &fsp_pkt);

    uint8_t encoded_frame[FSP_PKT_MAX_LENGTH];
    uint8_t frame_len;

    frame_encode(&fsp_pkt, encoded_frame, &frame_len);

  //  SCH_Delay(5);
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

int Cmd_pmu_get_all(int argc, char *argv[])
{
    if ((argc-1) < 1) return CMDLINE_TOO_FEW_ARGS;
    if ((argc-1) > 1) return CMDLINE_TOO_MANY_ARGS;
    // BA
    /*
:  --> 00   -> PDU
:  --> 01   -> PMU (*)
:  --> 10   -> CAM
:  --> 11   -> IOU
     */
    LL_GPIO_ResetOutputPin(GPIOA, BOARD_SEL_B_Pin);
    LL_GPIO_SetOutputPin(GPIOA, BOARD_SEL_A_Pin);
    USART_TypeDef* USARTx = (USART_TypeDef*)argv[argc-1];
    if (USARTx == UART5) {
    	uart_choose_uart5 = 1;
    }else{
    	uart_choose_uart5 = 0;
    }
    uint8_t cmd  = CMD_CODE_PMU_ALL;
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

