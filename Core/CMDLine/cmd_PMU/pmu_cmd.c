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

#define DEST_ADDR FSP_ADR_PMU

int Cmd_pmu_get_temp(int argc, char *argv[])
{
    if (argc < 1) return CMDLINE_TOO_FEW_ARGS;
    if (argc > 1) return CMDLINE_TOO_MANY_ARGS;

    // BA
    /*
:  --> 00   -> PDU
:  --> 01   -> PMU (*)
:  --> 10   -> CAM
:  --> 11   -> IOU
     */
    LL_GPIO_ResetOutputPin(GPIOA, BOARD_SEL_B_Pin);
    LL_GPIO_SetOutputPin(GPIOA, BOARD_SEL_A_Pin);

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
    if (argc < 1) return CMDLINE_TOO_FEW_ARGS;
    if (argc > 1) return CMDLINE_TOO_MANY_ARGS;
    // BA
    /*
:  --> 00   -> PDU
:  --> 01   -> PMU (*)
:  --> 10   -> CAM
:  --> 11   -> IOU
     */
    LL_GPIO_ResetOutputPin(GPIOA, BOARD_SEL_B_Pin);
    LL_GPIO_SetOutputPin(GPIOA, BOARD_SEL_A_Pin);

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
    if (argc < 1) return CMDLINE_TOO_FEW_ARGS;
    if (argc > 1) return CMDLINE_TOO_MANY_ARGS;

    // BA
    /*
:  --> 00   -> PDU
:  --> 01   -> PMU (*)
:  --> 10   -> CAM
:  --> 11   -> IOU
     */
    LL_GPIO_ResetOutputPin(GPIOA, BOARD_SEL_B_Pin);
    LL_GPIO_SetOutputPin(GPIOA, BOARD_SEL_A_Pin);

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
    if (argc < 1) return CMDLINE_TOO_FEW_ARGS;
    if (argc > 1) return CMDLINE_TOO_MANY_ARGS;
    // BA
    /*
:  --> 00   -> PDU
:  --> 01   -> PMU (*)
:  --> 10   -> CAM
:  --> 11   -> IOU
     */
    LL_GPIO_ResetOutputPin(GPIOA, BOARD_SEL_B_Pin);
    LL_GPIO_SetOutputPin(GPIOA, BOARD_SEL_A_Pin);

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
    if (argc < 3) return CMDLINE_TOO_FEW_ARGS;
    if (argc > 3) return CMDLINE_TOO_MANY_ARGS;
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
    if (argc < 2) return CMDLINE_TOO_FEW_ARGS;
    if (argc > 2) return CMDLINE_TOO_MANY_ARGS;
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
    if (argc < 2) return CMDLINE_TOO_FEW_ARGS;
    if (argc > 2) return CMDLINE_TOO_MANY_ARGS;
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
    if (argc < 1) return CMDLINE_TOO_FEW_ARGS;
    if (argc > 1) return CMDLINE_TOO_MANY_ARGS;
    // BA
    /*
:  --> 00   -> PDU
:  --> 01   -> PMU (*)
:  --> 10   -> CAM
:  --> 11   -> IOU
     */
    LL_GPIO_ResetOutputPin(GPIOA, BOARD_SEL_B_Pin);
    LL_GPIO_SetOutputPin(GPIOA, BOARD_SEL_A_Pin);

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

