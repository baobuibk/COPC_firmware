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

#define DEST_ADDR FSP_ADR_PDU

int Cmd_pdu_set_channel(int argc, char *argv[])
{
    if (argc < 3) return CMDLINE_TOO_FEW_ARGS;
    if (argc > 3) return CMDLINE_TOO_MANY_ARGS;
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
    if (argc < 3) return CMDLINE_TOO_FEW_ARGS;
    if (argc > 3) return CMDLINE_TOO_MANY_ARGS;
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
    if (argc < 2) return CMDLINE_TOO_FEW_ARGS;
    if (argc > 2) return CMDLINE_TOO_MANY_ARGS;

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
    if (argc < 2) return CMDLINE_TOO_FEW_ARGS;
    if (argc > 2) return CMDLINE_TOO_MANY_ARGS;
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
    if (argc < 2) return CMDLINE_TOO_FEW_ARGS;
    if (argc > 2) return CMDLINE_TOO_MANY_ARGS;
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
    if (argc < 1) return CMDLINE_TOO_FEW_ARGS;
    if (argc > 1) return CMDLINE_TOO_MANY_ARGS;

    // BA
    /*
:  --> 00   -> PDU (*)
:  --> 01   -> PMU
:  --> 10   -> CAM
:  --> 11   -> IOU
     */
    LL_GPIO_ResetOutputPin(GPIOA, BOARD_SEL_B_Pin);
    LL_GPIO_ResetOutputPin(GPIOA, BOARD_SEL_A_Pin);

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
