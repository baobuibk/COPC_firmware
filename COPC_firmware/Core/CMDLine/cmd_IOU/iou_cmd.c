/*
 * iou.c
 *
 *  Created on: May 26, 2024
 *      Author: CAO HIEU
 */
#include "../../ThirdParty/libfsp/fsp.h"
#include "iou_cmd.h"
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
//*****************************************************************************
//
// Function: set_temp
// Cmd code: 1
// Description: set the setpoint temperature
// Type: FSP_PKT_TYPE_CMD_WITH_ACK
// Payload: uint8 cmd
//          uint8 channel
//          uint16 setpoint
//
//*****************************************************************************
//
// Function: get_temp
// Cmd code: 2
// Description: get temperature of channel
// Type: FSP_PKT_TYPE_CMD_WITH_ACK
// Payload: uint8 cmd
//          uint8 channel
//          uint8 sensor (0: NTC, 1: one wire, 2: I2C sensor)
//
//*****************************************************************************
//
// Function: get_temp_setpoint
// Cmd code: 3
// Description: get the setpoint of channel
// Type: FSP_PKT_TYPE_CMD_WITH_ACK
// Payload: uint8 cmd
//          uint16 channel
//
//*****************************************************************************
//
// Function: Tec_ena
// Cmd code: 4
// Description: enable tec channel
// Type: FSP_PKT_TYPE_CMD_WITH_ACK
// Payload: uint8 cmd
//          uint8 channel
//
//*****************************************************************************
//
// Function: tec_dis
// Cmd code: 5
// Description: disable tec channel
// Type: FSP_PKT_TYPE_CMD_WITH_ACK
// Payload: uint8 cmd
//          uint8 channel
//
//*****************************************************************************
//
// Function: tec_dis_auto
// Cmd code: 6
// Description: disable tec auto control
// Type: FSP_PKT_TYPE_CMD_WITH_ACK
// Payload: uint8 cmd
//          uint8 channel
//
//*****************************************************************************
//
// Function: tec_ena_auto
// Cmd code: 7
// Description: enable tec auto control
// Type: FSP_PKT_TYPE_CMD_WITH_ACK
// Payload: uint8 cmd
//          uint8 channel
//
//*****************************************************************************
//
// Function: tec_set_output
// Cmd code: 8
// Description: set tec_output
// Type: FSP_PKT_TYPE_CMD_WITH_ACK
// Payload: uint8 cmd
//          uint8_t channel (7 bit for channel)
//          uint8_t heat_cool (1: heat, 0: cool)
//          uint16_t volatge out (250 mean 2.5V)
//
//*****************************************************************************
//
// Function: tec_set_PID_param
// Description: set PID param
// Type: FSP_PKT_TYPE_CMD_WITH_ACK
// Payload: uint8 cmd
//          uint16_t KP
//          uint16_t KI
//          uint16_t KD
//
//*****************************************************************************
//
// Function: tec_get_PID_param
// Description: get PID param
// Type: FSP_PKT_TYPE_CMD_WITH_ACK
// Payload: uint16_t KD
//
//*****************************************************************************
//
// Function: ir_led_set
// Cmd code: 9
// Type: FSP_PKT_TYPE_CMD_WITH_ACK
// Payload: uint8 cmd
//          uint8 duty (0 to 100)
//
//*****************************************************************************
//

// Function: ir_led_off
// Cmd code: 10
// Type: FSP_PKT_TYPE_CMD_WITH_ACK
// Payload: uint8 cmd
//
//*****************************************************************************
//
// Function: neo_set_rgb
// Cmd code: 11
// Type: FSP_PKT_TYPE_CMD_WITH_ACK
// Payload: uint8 cmd
//          uint8 R
//          uint8 G
//          uint8 B
//
//*****************************************************************************
//
// Function: neo_set_bright
// Cmd code: 14
// Type: FSP_PKT_TYPE_CMD_WITH_ACK
// Payload: uint8 cmd
//          uint8 brightness
//
//*****************************************************************************
//
// Function: iou_get_status
// Cmd code: 15
// Description: get IOU status
// Type: FSP_PKT_TYPE_CMD_WITH_ACK
// Payload: uint8 cmd
//
//*****************************************************************************
#define DEST_ADDR FSP_ADR_IOU

uint8_t iou_frame[] = {0xCA, 0x01, 0x05, 0x01, 0x04, 0x13, 0xCF, 0xB2, 0xEF};

#define IOU_PERIOD 1000

static void IOU_update_task(void);


typedef struct IOU_TaskContextTypedef
{
	SCH_TASK_HANDLE               taskHandle;
	SCH_TaskPropertyTypedef       taskProperty;
} IOU_TaskContextTypedef;


static IOU_TaskContextTypedef           IOU_task_context =
{
	SCH_INVALID_TASK_HANDLE,                 // Will be updated by Schedular
	{
		SCH_TASK_SYNC,                      // taskType;
		SCH_TASK_PRIO_0,                    // taskPriority;
		100,                                // taskPeriodInMS;
		IOU_update_task					// taskFunction;
	}
};



void IOU_create_task(void)
{
    SCH_TASK_CreateTask(&IOU_task_context.taskHandle, &IOU_task_context.taskProperty);
    SCH_TIM_Start(SCH_TIM_IOU, IOU_PERIOD);
    Ringbuf_init();
}

volatile uint8_t receive_pduFlag = 1;
volatile uint8_t receive_pmuFlag = 1;
volatile uint8_t receive_iouFlag = 1;
volatile uint8_t disconnect_counter_iou;
volatile uint8_t disconnect_counter_pdu;
volatile uint8_t disconnect_counter_pmu;
volatile uint8_t send_rs422 = 0;

volatile uint8_t timeout_counter_iou = 0;


void IOU_update_task(void) {
	if (auto_report_enabled) {

//	if  not in send and wait

		uint8_t *frame;
		uint8_t frame_len;
		if (SCH_TIM_HasCompleted(SCH_TIM_IOU))
		{

			if(!sendFlag){
				if(!send_rs422){
					if(receive_pduFlag&&receive_pmuFlag){
						switch_board(0);
						Uart_flush(USART1);

						frame = iou_frame;
						frame_len = sizeof(iou_frame);
						for (int i = 0; i < frame_len; i++) {
							Uart_write(USART1, frame[i]);
						}
						receive_iouFlag = 0;
						send_rs422 = 1;
						SCH_TIM_Start(SCH_TIM_IOU, IOU_PERIOD);
					}
					if(!receive_iouFlag){
						timeout_counter_iou++;
						if (timeout_counter_iou > 2){
							disconnect_counter_iou++;
							timeout_counter_iou = 0;
							receive_pmuFlag = 1;
							if(disconnect_counter_iou> 4){
								for (int i = 1; i <= 35; i++) {
										    sourceArray[i + 7] = 0xFF; //42   =  35  + 7      8 -> pay 1   9 -> pay2    43 -< pay35
								}
							}
						}
					}
				}
			}
		}
	}
}


void	iou_create_task(void)
{
	SCH_TASK_CreateTask(&IOU_task_context.taskHandle, &IOU_task_context.taskProperty);
	Ringbuf_init();
}


//int TEST(int argc, char *argv[])
//{
//    if (argc < 3) return CMDLINE_TOO_FEW_ARGS;
//    if (argc > 3) return CMDLINE_TOO_MANY_ARGS;
//
//    uint8_t channel = atoi(argv[1]);
//    if (channel > 4)    return CMDLINE_INVALID_ARG;
//
//    uint16_t setpoint = atoi(argv[2]);
//
//    // Create the command payload
////    uint8_t payload[4];
////    payload[0] = 0x01;  // Command ID for set_temp
////    payload[1] = channel;
////    payload[2] = (uint8_t)(setpoint >> 8);  // setpoint high byte
////    payload[3] = (uint8_t)(setpoint & 0xFF); // setpoint low byte
//
////    // Generate the FSP packet
////    fsp_packet_t fsp_pkt;
////  //  fsp_gen_pkt(payload, sizeof(payload), DEST_ADDR, FSP_PKT_TYPE_DATA_WITH_ACK, &fsp_pkt);
////
////    // Encode the FSP packet
////    uint8_t encoded_pkt[FSP_PKT_MAX_LENGTH];
////    uint8_t encoded_len;
////    fsp_encode(&fsp_pkt, encoded_pkt, &encoded_len);
//
//    // Send the encoded packet over the communication interface
//    //   send_data(encoded_pkt, encoded_len);
//
//    Uart_sendstring(USART1, "\r\n> Sending, please wait for response... ");
//    LL_GPIO_SetOutputPin(GPIOA, BOARD_SEL_A_Pin|BOARD_SEL_B_Pin);
//    SCH_Delay(5);
////
////    set_fsp_packet(encoded_pkt, encoded_len);
////    set_send_flag();
//
////    if (ack_received) {
////        Uart_sendstring(USART1, "ACK\r\n");
////    } else {
////        Uart_sendstring(USART1, "TIMEOUT_NORESPONE\r\n");
////    }
//
////    ==>>>> CALL 1 ham cho` ack -> Xong ack la ok
////    ==>>>> khi gui thi mo cong ra, + delay vai ms -> Set co`
////	==>>>> Neu vay thi luon luon mo cong, khi mà nhắn bên 1 2 3 4, nhắn bên 1 thì mở luôn bên 1
////	==>>>> Nhắn bên 2 thì mở luôn bên 2 nếu mà chưa nhận được ack -> busy please wait (processing...)
////    ==>>>> Đoạn đấy bỏ vào trong timeout -> Sau đó mới hiện ok, hoặc done gì đó
//
//
//    return CMDLINE_PENDING;
//}

volatile uint8_t uart_choose_uart5 = 0;

int Cmd_iou_set_temp(int argc, char *argv[])
{


    if ((argc-1) < 3) return CMDLINE_TOO_FEW_ARGS;
    if ((argc-1) > 3) return CMDLINE_TOO_MANY_ARGS;

    uint8_t channel = atoi(argv[1]);
    if (channel > 3)    return CMDLINE_INVALID_ARG;

    uint16_t temp = atoi(argv[2]);
    if (temp > 500)    return CMDLINE_INVALID_ARG;


    // BA
    /*
:  --> 00   -> PDU
:  --> 01   -> PMU
:  --> 10   -> CAM
:  --> 11   -> IOU (*)
     */
    LL_GPIO_SetOutputPin(GPIOA, BOARD_SEL_B_Pin);
    LL_GPIO_SetOutputPin(GPIOA, BOARD_SEL_A_Pin);
    USART_TypeDef* USARTx = (USART_TypeDef*)argv[argc-1];
    if (USARTx == UART5) {
    	uart_choose_uart5 = 1;
    }else{
    	uart_choose_uart5 = 0;
    }

    // Create the command payload
    uint8_t cmd  = CMD_CODE_SET_TEMP;
    uint8_t payload[3];
    payload[0]  = channel;
    payload[1]  = (uint8_t)(temp >> 8);   //high
    payload[2]  = (uint8_t)(temp & 0xFF); //low
    fsp_packet_t  fsp_pkt;
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

// //   SCH_Delay(5);
//    set_fsp_packet(encoded_frame, frame_len);
//    set_send_flag();

//  ==>>>> CALL 1 ham cho` ack -> Xong ack la ok
//  ==>>>> khi gui thi mo cong ra, + delay vai ms -> Set co`
//	==>>>> Neu vay thi luon luon mo cong, khi mà nhắn bên 1 2 3 4, nhắn bên 1 thì mở luôn bên 1
//	==>>>> Nhắn bên 2 thì mở luôn bên 2 nếu mà chưa nhận được ack -> busy please wait (processing...)
//  ==>>>> Đoạn đấy bỏ vào trong timeout -> Sau đó mới hiện ok, hoặc done gì đó

    return CMDLINE_PENDING;
}


int Cmd_iou_get_temp(int argc, char *argv[])
{
    if ((argc-1) < 3) return CMDLINE_TOO_FEW_ARGS;
    if ((argc-1) > 3) return CMDLINE_TOO_MANY_ARGS;


    uint8_t sensor = atoi(argv[1]);
    if (sensor > 1)    return CMDLINE_INVALID_ARG;

    uint8_t channel = atoi(argv[2]);
    if (channel > 3)    return CMDLINE_INVALID_ARG;

    // BA
    /*
:  --> 00   -> PDU
:  --> 01   -> PMU
:  --> 10   -> CAM
:  --> 11   -> IOU (*)
     */
    LL_GPIO_SetOutputPin(GPIOA, BOARD_SEL_B_Pin);
    LL_GPIO_SetOutputPin(GPIOA, BOARD_SEL_A_Pin);
    USART_TypeDef* USARTx = (USART_TypeDef*)argv[argc-1];
    if (USARTx == UART5) {
    	uart_choose_uart5 = 1;
    }else{
    	uart_choose_uart5 = 0;
    }
    uint8_t cmd  = CMD_CODE_GET_TEMP;
    uint8_t payload[2];


    payload[0] = sensor;
    payload[1] = channel;

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

int Cmd_iou_temp_setpoint(int argc, char *argv[])
{
    if ((argc-1) < 2) return CMDLINE_TOO_FEW_ARGS;
    if ((argc-1) > 2) return CMDLINE_TOO_MANY_ARGS;
    uint8_t channel = atoi(argv[1]);
    if (channel > 3)    return CMDLINE_INVALID_ARG;

    // BA
    /*
:  --> 00   -> PDU
:  --> 01   -> PMU
:  --> 10   -> CAM
:  --> 11   -> IOU (*)
     */
    LL_GPIO_SetOutputPin(GPIOA, BOARD_SEL_B_Pin);
    LL_GPIO_SetOutputPin(GPIOA, BOARD_SEL_A_Pin);
    USART_TypeDef* USARTx = (USART_TypeDef*)argv[argc-1];
    if (USARTx == UART5) {
    	uart_choose_uart5 = 1;
    }else{
    	uart_choose_uart5 = 0;
    }
    uint8_t cmd  = CMD_CODE_TEMP_SETPOINT;
    uint8_t payload[1];
    payload[0] = channel;

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

int Cmd_iou_tec_ena(int argc, char *argv[])
{
    if ((argc-1) < 2) return CMDLINE_TOO_FEW_ARGS;
    if ((argc-1) > 2) return CMDLINE_TOO_MANY_ARGS;
    uint8_t channel = atoi(argv[1]);
    if (channel > 3)    return CMDLINE_INVALID_ARG;
    // BA
    /*
:  --> 00   -> PDU
:  --> 01   -> PMU
:  --> 10   -> CAM
:  --> 11   -> IOU (*)
     */
    LL_GPIO_SetOutputPin(GPIOA, BOARD_SEL_B_Pin);
    LL_GPIO_SetOutputPin(GPIOA, BOARD_SEL_A_Pin);
    USART_TypeDef* USARTx = (USART_TypeDef*)argv[argc-1];
    if (USARTx == UART5) {
    	uart_choose_uart5 = 1;
    }else{
    	uart_choose_uart5 = 0;
    }
    uint8_t cmd  = CMD_CODE_TEC_ENA;
    uint8_t payload[1];
    payload[0] = channel;

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

int Cmd_iou_tec_dis(int argc, char *argv[])
{
    if ((argc-1) < 2) return CMDLINE_TOO_FEW_ARGS;
    if ((argc-1) > 2) return CMDLINE_TOO_MANY_ARGS;
    uint8_t channel = atoi(argv[1]);
    if (channel > 3)    return CMDLINE_INVALID_ARG;

    // BA
    /*
:  --> 00   -> PDU
:  --> 01   -> PMU
:  --> 10   -> CAM
:  --> 11   -> IOU (*)
     */
    LL_GPIO_SetOutputPin(GPIOA, BOARD_SEL_B_Pin);
    LL_GPIO_SetOutputPin(GPIOA, BOARD_SEL_A_Pin);
    USART_TypeDef* USARTx = (USART_TypeDef*)argv[argc-1];
    if (USARTx == UART5) {
    	uart_choose_uart5 = 1;
    }else{
    	uart_choose_uart5 = 0;
    }

    uint8_t cmd  = CMD_CODE_TEC_DIS;
    uint8_t payload[1];
    payload[0] = channel;

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

int Cmd_iou_tec_ena_auto(int argc, char *argv[])
{
    if ((argc-1) < 2) return CMDLINE_TOO_FEW_ARGS;
    if ((argc-1) > 2) return CMDLINE_TOO_MANY_ARGS;
    uint8_t channel = atoi(argv[1]);
    if (channel > 3)    return CMDLINE_INVALID_ARG;

    // BA
    /*
:  --> 00   -> PDU
:  --> 01   -> PMU
:  --> 10   -> CAM
:  --> 11   -> IOU (*)
     */
    LL_GPIO_SetOutputPin(GPIOA, BOARD_SEL_B_Pin);
    LL_GPIO_SetOutputPin(GPIOA, BOARD_SEL_A_Pin);
    USART_TypeDef* USARTx = (USART_TypeDef*)argv[argc-1];
    if (USARTx == UART5) {
    	uart_choose_uart5 = 1;
    }else{
    	uart_choose_uart5 = 0;
    }
    uint8_t cmd  = CMD_CODE_TEC_ENA_AUTO;
    uint8_t payload[1];
    payload[0] = channel;

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

int Cmd_iou_tec_dis_auto(int argc, char *argv[])
{
    if ((argc-1) < 2) return CMDLINE_TOO_FEW_ARGS;
    if ((argc-1) > 2) return CMDLINE_TOO_MANY_ARGS;
    uint8_t channel = atoi(argv[1]);
    if (channel > 3)    return CMDLINE_INVALID_ARG;
    // BA
    /*
:  --> 00   -> PDU
:  --> 01   -> PMU
:  --> 10   -> CAM
:  --> 11   -> IOU (*)
     */
    LL_GPIO_SetOutputPin(GPIOA, BOARD_SEL_B_Pin);
    LL_GPIO_SetOutputPin(GPIOA, BOARD_SEL_A_Pin);
    USART_TypeDef* USARTx = (USART_TypeDef*)argv[argc-1];
    if (USARTx == UART5) {
    	uart_choose_uart5 = 1;
    }else{
    	uart_choose_uart5 = 0;
    }
    uint8_t cmd  = CMD_CODE_TEC_DIS_AUTO;
    uint8_t payload[1];
    payload[0] = channel;

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


int Cmd_iou_tec_set_output(int argc, char *argv[])
{
    if ((argc-1) < 4) return CMDLINE_TOO_FEW_ARGS;
    if ((argc-1) > 4) return CMDLINE_TOO_MANY_ARGS;
    uint8_t channel = atoi(argv[1]);
    if (channel > 3)    return CMDLINE_INVALID_ARG;

    uint8_t mode = atoi(argv[2]);
    if (mode > 1)    return CMDLINE_INVALID_ARG;

    uint16_t vol = atoi(argv[3]);
    if (vol > 500)    return CMDLINE_INVALID_ARG;

    // BA
    /*
:  --> 00   -> PDU
:  --> 01   -> PMU
:  --> 10   -> CAM
:  --> 11   -> IOU (*)
     */
    LL_GPIO_SetOutputPin(GPIOA, BOARD_SEL_B_Pin);
    LL_GPIO_SetOutputPin(GPIOA, BOARD_SEL_A_Pin);
    USART_TypeDef* USARTx = (USART_TypeDef*)argv[argc-1];
    if (USARTx == UART5) {
    	uart_choose_uart5 = 1;
    }else{
    	uart_choose_uart5 = 0;
    }
    uint8_t cmd  = CMD_CODE_TEC_SET_OUTPUT;
    uint8_t payload[4];
    payload[0] = channel;
    payload[1] = mode;
    payload[2]  = (uint8_t)(vol >> 8);   //high
    payload[3]  = (uint8_t)(vol & 0xFF); //low

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

int Cmd_iou_tec_auto_vol(int argc, char *argv[])
{
    if ((argc-1) < 3) return CMDLINE_TOO_FEW_ARGS;
    if ((argc-1) > 3) return CMDLINE_TOO_MANY_ARGS;
    uint8_t channel = atoi(argv[1]);
    if (channel > 3)    return CMDLINE_INVALID_ARG;

    uint16_t vol = atoi(argv[2]);
    if (vol > 500)    return CMDLINE_INVALID_ARG;

    // BA
    /*
:  --> 00   -> PDU
:  --> 01   -> PMU
:  --> 10   -> CAM
:  --> 11   -> IOU (*)
     */
    LL_GPIO_SetOutputPin(GPIOA, BOARD_SEL_B_Pin);
    LL_GPIO_SetOutputPin(GPIOA, BOARD_SEL_A_Pin);
    USART_TypeDef* USARTx = (USART_TypeDef*)argv[argc-1];
    if (USARTx == UART5) {
    	uart_choose_uart5 = 1;
    }else{
    	uart_choose_uart5 = 0;
    }
    uint8_t cmd  = CMD_CODE_TEC_AUTO_VOL;
    uint8_t payload[3];
    payload[0] = channel;
    payload[1]  = (uint8_t)(vol >> 8);   //high
    payload[2]  = (uint8_t)(vol & 0xFF); //low

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

int Cmd_iou_tec_status(int argc, char *argv[])
{
    if ((argc-1) < 1) return CMDLINE_TOO_FEW_ARGS;
    if ((argc-1) > 1) return CMDLINE_TOO_MANY_ARGS;

    // BA
    /*
:  --> 00   -> PDU
:  --> 01   -> PMU
:  --> 10   -> CAM
:  --> 11   -> IOU (*)
     */
    LL_GPIO_SetOutputPin(GPIOA, BOARD_SEL_B_Pin);
    LL_GPIO_SetOutputPin(GPIOA, BOARD_SEL_A_Pin);
    USART_TypeDef* USARTx = (USART_TypeDef*)argv[argc-1];
    if (USARTx == UART5) {
    	uart_choose_uart5 = 1;
    }else{
    	uart_choose_uart5 = 0;
    }
    uint8_t cmd  = CMD_CODE_TEC_STATUS;
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

int Cmd_iou_tec_log_ena(int argc, char *argv[])
{
    if ((argc-1) < 1) return CMDLINE_TOO_FEW_ARGS;
    if ((argc-1) > 1) return CMDLINE_TOO_MANY_ARGS;
    // BA
    /*
:  --> 00   -> PDU
:  --> 01   -> PMU
:  --> 10   -> CAM
:  --> 11   -> IOU (*)
     */
    LL_GPIO_SetOutputPin(GPIOA, BOARD_SEL_B_Pin);
    LL_GPIO_SetOutputPin(GPIOA, BOARD_SEL_A_Pin);
    USART_TypeDef* USARTx = (USART_TypeDef*)argv[argc-1];
    if (USARTx == UART5) {
    	uart_choose_uart5 = 1;
    }else{
    	uart_choose_uart5 = 0;
    }
    uint8_t cmd  = CMD_CODE_TEC_LOG_ENA;
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

int Cmd_iou_tec_log_dis(int argc, char *argv[])
{
    if ((argc-1) < 1) return CMDLINE_TOO_FEW_ARGS;
    if ((argc-1) > 1) return CMDLINE_TOO_MANY_ARGS;
    // BA
    /*
:  --> 00   -> PDU
:  --> 01   -> PMU
:  --> 10   -> CAM
:  --> 11   -> IOU (*)
     */
    LL_GPIO_SetOutputPin(GPIOA, BOARD_SEL_B_Pin);
    LL_GPIO_SetOutputPin(GPIOA, BOARD_SEL_A_Pin);
    USART_TypeDef* USARTx = (USART_TypeDef*)argv[argc-1];
    if (USARTx == UART5) {
    	uart_choose_uart5 = 1;
    }else{
    	uart_choose_uart5 = 0;
    }
    uint8_t cmd  = CMD_CODE_TEC_LOG_DIS;
    fsp_packet_t fsp_pkt;

    fsp_gen_cmd_pkt(cmd, DEST_ADDR, FSP_PKT_WITH_ACK, &fsp_pkt);

    uint8_t encoded_frame[FSP_PKT_MAX_LENGTH];
    uint8_t frame_len;

    frame_encode(&fsp_pkt, encoded_frame, &frame_len);

//    SCH_Delay(5);
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

int Cmd_iou_ringled_setRGB(int argc, char *argv[])
{
    if ((argc-1) < 5) return CMDLINE_TOO_FEW_ARGS;
    if ((argc-1) > 5) return CMDLINE_TOO_MANY_ARGS;
    uint8_t red = atoi(argv[1]);
    if (red > 255)    return CMDLINE_INVALID_ARG;
    uint8_t green = atoi(argv[2]);
    if (green > 255)    return CMDLINE_INVALID_ARG;
    uint8_t blue = atoi(argv[3]);
    if (blue > 255)    return CMDLINE_INVALID_ARG;
    uint8_t white = atoi(argv[4]);
    if (white > 255)    return CMDLINE_INVALID_ARG;
    // BA
    /*
:  --> 00   -> PDU
:  --> 01   -> PMU
:  --> 10   -> CAM
:  --> 11   -> IOU (*)
     */
    LL_GPIO_SetOutputPin(GPIOA, BOARD_SEL_B_Pin);
    LL_GPIO_SetOutputPin(GPIOA, BOARD_SEL_A_Pin);
    USART_TypeDef* USARTx = (USART_TypeDef*)argv[argc-1];
    if (USARTx == UART5) {
    	uart_choose_uart5 = 1;
    }else{
    	uart_choose_uart5 = 0;
    }
    uint8_t cmd  = CMD_CODE_RINGLED_SETRGB;
    uint8_t payload[4];

    payload[0]  = red; //low
    payload[1]  = green; //low
    payload[2]  = blue; //low
    payload[3]  = white; //low
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

int Cmd_iou_ringled_getRGB(int argc, char *argv[])
{
    if ((argc-1) < 1) return CMDLINE_TOO_FEW_ARGS;
    if ((argc-1) > 1) return CMDLINE_TOO_MANY_ARGS;

    // BA
    /*
:  --> 00   -> PDU
:  --> 01   -> PMU
:  --> 10   -> CAM
:  --> 11   -> IOU (*)
     */
    LL_GPIO_SetOutputPin(GPIOA, BOARD_SEL_B_Pin);
    LL_GPIO_SetOutputPin(GPIOA, BOARD_SEL_A_Pin);
    USART_TypeDef* USARTx = (USART_TypeDef*)argv[argc-1];
    if (USARTx == UART5) {
    	uart_choose_uart5 = 1;
    }else{
    	uart_choose_uart5 = 0;
    }

    uint8_t cmd  = CMD_CODE_RINGLED_GETRGB;

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

int Cmd_iou_irled_set_bright(int argc, char *argv[])
{
    if ((argc-1) < 2) return CMDLINE_TOO_FEW_ARGS;
    if ((argc-1) > 2) return CMDLINE_TOO_MANY_ARGS;
    uint8_t percent = atoi(argv[1]);
    if (percent > 100)    return CMDLINE_INVALID_ARG;

    // BA
    /*
:  --> 00   -> PDU
:  --> 01   -> PMU
:  --> 10   -> CAM
:  --> 11   -> IOU (*)
     */
    LL_GPIO_SetOutputPin(GPIOA, BOARD_SEL_B_Pin);
    LL_GPIO_SetOutputPin(GPIOA, BOARD_SEL_A_Pin);
    USART_TypeDef* USARTx = (USART_TypeDef*)argv[argc-1];
    if (USARTx == UART5) {
    	uart_choose_uart5 = 1;
    }else{
    	uart_choose_uart5 = 0;
    }
    uint8_t cmd  = CMD_CODE_IRLED_SET_BRIGHT;
    uint8_t payload[1];

    payload[0]  = percent; //low

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

int Cmd_iou_irled_get_bright(int argc, char *argv[])
{
    if ((argc-1) < 1) return CMDLINE_TOO_FEW_ARGS;
    if ((argc-1) > 1) return CMDLINE_TOO_MANY_ARGS;
    // BA
    /*
:  --> 00   -> PDU
:  --> 01   -> PMU
:  --> 10   -> CAM
:  --> 11   -> IOU (*)
     */
    LL_GPIO_SetOutputPin(GPIOA, BOARD_SEL_B_Pin);
    LL_GPIO_SetOutputPin(GPIOA, BOARD_SEL_A_Pin);
    USART_TypeDef* USARTx = (USART_TypeDef*)argv[argc-1];
    if (USARTx == UART5) {
    	uart_choose_uart5 = 1;
    }else{
    	uart_choose_uart5 = 0;
    }
    uint8_t cmd  = CMD_CODE_IRLED_GET_BRIGHT;
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



int Cmd_iou_get_accel(int argc, char *argv[])
{
    if ((argc-1) < 1) return CMDLINE_TOO_FEW_ARGS;
    if ((argc-1) > 1) return CMDLINE_TOO_MANY_ARGS;
    // BA
    /*
:  --> 00   -> PDU
:  --> 01   -> PMU
:  --> 10   -> CAM
:  --> 11   -> IOU (*)
     */
    LL_GPIO_SetOutputPin(GPIOA, BOARD_SEL_B_Pin);
    LL_GPIO_SetOutputPin(GPIOA, BOARD_SEL_A_Pin);
    USART_TypeDef* USARTx = (USART_TypeDef*)argv[argc-1];
    if (USARTx == UART5) {
    	uart_choose_uart5 = 1;
    }else{
    	uart_choose_uart5 = 0;
    }
    uint8_t cmd  = CMD_CODE_GET_ACCEL_GYRO;
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

int Cmd_iou_get_press(int argc, char *argv[])
{
    if ((argc-1) < 1) return CMDLINE_TOO_FEW_ARGS;
    if ((argc-1) > 1) return CMDLINE_TOO_MANY_ARGS;

    // BA
    /*
:  --> 00   -> PDU
:  --> 01   -> PMU
:  --> 10   -> CAM
:  --> 11   -> IOU (*)
     */
    LL_GPIO_SetOutputPin(GPIOA, BOARD_SEL_B_Pin);
    LL_GPIO_SetOutputPin(GPIOA, BOARD_SEL_A_Pin);
    USART_TypeDef* USARTx = (USART_TypeDef*)argv[argc-1];
    if (USARTx == UART5) {
    	uart_choose_uart5 = 1;
    }else{
    	uart_choose_uart5 = 0;
    }
    uint8_t cmd  = CMD_CODE_GET_PRESS;
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


int Cmd_iou_get_parameters(int argc, char *argv[])
{
    if ((argc-1) < 1) return CMDLINE_TOO_FEW_ARGS;
    if ((argc-1) > 1) return CMDLINE_TOO_MANY_ARGS;

    // BA
    /*
:  --> 00   -> PDU
:  --> 01   -> PMU
:  --> 10   -> CAM
:  --> 11   -> IOU (*)
     */
    LL_GPIO_SetOutputPin(GPIOA, BOARD_SEL_B_Pin);
    LL_GPIO_SetOutputPin(GPIOA, BOARD_SEL_A_Pin);
    USART_TypeDef* USARTx = (USART_TypeDef*)argv[argc-1];
    if (USARTx == UART5) {
    	uart_choose_uart5 = 1;
    }else{
    	uart_choose_uart5 = 0;
    }
    uint8_t cmd  = CMD_CODE_GET_PARAMETERS;
    fsp_packet_t fsp_pkt;

    fsp_gen_cmd_pkt(cmd, DEST_ADDR, FSP_PKT_WITH_ACK, &fsp_pkt);

    uint8_t encoded_frame[FSP_PKT_MAX_LENGTH];
    uint8_t frame_len;

    frame_encode(&fsp_pkt, encoded_frame, &frame_len);

  //  SCH_Delay(5);.
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



volatile uint8_t ack_received = 0;

//int send_packet_and_wait_for_ack(uint8_t *encoded_pkt, uint8_t encoded_len, uint32_t timeout_ms)
//{
//    int max_retries = 3;  // Maximum number of retries
//    int retry_count = 0;  // Current retry count
//
//    ack_received = 0;
//    while (retry_count < max_retries) {
//        // Send the encoded packet
//        for (int i = 0; i < encoded_len; i++) {
//            Uart_write(UART5, encoded_pkt[i]);
//        }
//         // Reset cờ ACK trước khi gửi
//
//        // Wait for ACK with timeout
//        uint32_t start_time = g_systick_count;
//
//        while (!ack_received) {
//            // Calculate elapsed time
//            uint32_t elapsed_time = g_systick_count - start_time;
//
//            // Check if timeout has occurred
//            if (elapsed_time >= timeout_ms) {
//                break;
//            }
//        }
//
//        if (ack_received) {
//            return 1; // ACK received, return success
//        }
//
//        // Increment the retry count
//        retry_count++;
//    }
//
//    // Maximum retries reached, return failure
//    return 0;
//}
