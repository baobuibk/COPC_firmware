///*
// * cmd_cam.c
// *
// *  Created on: Jun 10, 2024
// *      Author: CAO HIEU
// */
////#include "../global_vars.h"
//
//#define DEST_ADDR FSP_ADR_CAM
//
//uint8_t cam_frame[] = {0xCA, 0x01, 0x04, 0x01, 0x04, 0x01, 0x8B, 0x75, 0xEF};
//
//#define CAM_PERIOD 1500
//
//static void CAM_update_task(void);
//
//
//typedef struct CAM_TaskContextTypedef
//{
//	SCH_TASK_HANDLE               taskHandle;
//	SCH_TaskPropertyTypedef       taskProperty;
//} CAM_TaskContextTypedef;
//
//
//static CAM_TaskContextTypedef           CAM_task_context =
//{
//	SCH_INVALID_TASK_HANDLE,                 // Will be updated by Schedular
//	{
//		SCH_TASK_SYNC,                      // taskType;
//		SCH_TASK_PRIO_0,                    // taskPriority;
//		100,                                // taskPeriodInMS;
//		CAM_update_task,					// taskFunction;
//		1477							//taskTick
//	}
//};
//
//
//
//void CAM_create_task(void)
//{
//    SCH_TASK_CreateTask(&CAM_task_context.taskHandle, &CAM_task_context.taskProperty);
//    SCH_TIM_Start(SCH_TIM_CAM, CAM_PERIOD);
//    Ringbuf_init();
//}
//
//
//
//
//void CAM_update_task(void) {
//	if (rs422_report_enable) {
////	if  not in send and wait
//		uint8_t *frame;
//		uint8_t frame_len;
//		if (SCH_TIM_HasCompleted(SCH_TIM_CAM))
//		{
//
//			if(!sendFlag){
//				if(!send_rs422){
//					if(receive_pduFlag&&receive_pmuFlag&&receive_iouFlag){
//						switch_board(2);
//						Uart_flush(USART1);
//
//						frame = cam_frame;
//						frame_len = sizeof(cam_frame);
//						for (int i = 0; i < frame_len; i++) {
//							Uart_write(USART1, frame[i]);
//						}
//						receive_camFlag = 0;
//						send_rs422 = 1;
//						send_by_cam = 1;
//						SCH_TIM_Start(SCH_TIM_CAM, CAM_PERIOD);
//					}
//				}else{
//					if(!receive_camFlag){
//						send_rs422 = 0;
//						receive_camFlag = 1;
//						send_by_cam = 0;
//						SCH_TIM_Start(SCH_TIM_CAM, CAM_PERIOD);
//
//						//Timeout memset 0xff
//					}
//				}
//			}
//		}
//	}
//}
//
//
//int Cmd_cam_get_data(int argc, char *argv[])
//{
//
//    if ((argc-1) < 1) return CMDLINE_TOO_FEW_ARGS;
//    if ((argc-1) > 1) return CMDLINE_TOO_MANY_ARGS;
//
//    // BA
//    /*
//:  --> 00   -> PDU
//:  --> 01   -> PMU
//:  --> 10   -> CAM (*)
//:  --> 11   -> IOU
//     */
//    LL_GPIO_SetOutputPin(GPIOA, BOARD_SEL_B_Pin);
//    LL_GPIO_ResetOutputPin(GPIOA, BOARD_SEL_A_Pin);
//    USART_TypeDef* USARTx = (USART_TypeDef*)argv[argc-1];
//    if (USARTx == UART5) {
//    	uart_choose_uart5 = 1;
//    }else{
//    	uart_choose_uart5 = 0;
//    }
//
//    if (USARTx == USART2) {
//    	uart_choose_usart2 = 1;
//    }else{
//    	uart_choose_usart2 = 0;
//    }
//
//    // Create the command payload
//    uint8_t cmd  = CMD_CODE_CAM_GET_DATA;
//
//    fsp_packet_t  fsp_pkt;
//    fsp_gen_cmd_pkt(cmd, DEST_ADDR, FSP_PKT_WITH_ACK, &fsp_pkt);
//
//    uint8_t encoded_frame[FSP_PKT_MAX_LENGTH];
//    uint8_t frame_len;
//
//    frame_encode(&fsp_pkt, encoded_frame, &frame_len);
//
//    if (frame_len > 0) {
//        for (int i = 0; i < frame_len; i++) {
//            Uart_write(USART1, encoded_frame[i]);
//
//        }
//        set_fsp_packet(encoded_frame, frame_len);
//        set_send_flag();
//    }else{
//    	fsp_gen_cmd_pkt(cmd, DEST_ADDR, FSP_PKT_WITH_ACK, &fsp_pkt);
//    	frame_encode(&fsp_pkt, encoded_frame, &frame_len);
//        set_fsp_packet(encoded_frame, frame_len);
//        set_send_flag();
//    }
//
//    return CMDLINE_PENDING;
//}
//
//int Cmd_cam_get_img(int argc, char *argv[])
//{
//
//    if ((argc-1) < 1) return CMDLINE_TOO_FEW_ARGS;
//    if ((argc-1) > 1) return CMDLINE_TOO_MANY_ARGS;
//
//    // BA
//    /*
//:  --> 00   -> PDU
//:  --> 01   -> PMU
//:  --> 10   -> CAM (*)
//:  --> 11   -> IOU
//     */
//    LL_GPIO_SetOutputPin(GPIOA, BOARD_SEL_B_Pin);
//    LL_GPIO_ResetOutputPin(GPIOA, BOARD_SEL_A_Pin);
//    USART_TypeDef* USARTx = (USART_TypeDef*)argv[argc-1];
//    if (USARTx == UART5) {
//    	uart_choose_uart5 = 1;
//    }else{
//    	uart_choose_uart5 = 0;
//    }
//
//    if (USARTx == USART2) {
//    	uart_choose_usart2 = 1;
//    }else{
//    	uart_choose_usart2 = 0;
//    }
//
//    // Create the command payload
//    uint8_t cmd  = CMD_CODE_CAM_GET_IMG;
//
//    fsp_packet_t  fsp_pkt;
//    fsp_gen_cmd_pkt(cmd, DEST_ADDR, FSP_PKT_WITH_ACK, &fsp_pkt);
//
//    uint8_t encoded_frame[FSP_PKT_MAX_LENGTH];
//    uint8_t frame_len;
//
//    frame_encode(&fsp_pkt, encoded_frame, &frame_len);
//
//    if (frame_len > 0) {
//        for (int i = 0; i < frame_len; i++) {
//            Uart_write(USART1, encoded_frame[i]);
//
//        }
//        set_fsp_packet(encoded_frame, frame_len);
//        set_send_flag();
//    }else{
//    	fsp_gen_cmd_pkt(cmd, DEST_ADDR, FSP_PKT_WITH_ACK, &fsp_pkt);
//    	frame_encode(&fsp_pkt, encoded_frame, &frame_len);
//        set_fsp_packet(encoded_frame, frame_len);
//        set_send_flag();
//    }
//
//    return CMDLINE_PENDING;
//}
//
