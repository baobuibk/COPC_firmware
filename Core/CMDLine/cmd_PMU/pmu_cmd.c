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

#define DEST_ADDR FSP_ADR_PMU

int Cmd_pmu_get_temp(int argc, char *argv[])
{
    if (argc < 1) return CMDLINE_TOO_FEW_ARGS;
    if (argc > 1) return CMDLINE_TOO_MANY_ARGS;

    uint8_t cmd  = CMD_CODE_PMU_GET_TEMP;
    fsp_packet_t fsp_pkt;

    fsp_gen_cmd_pkt(cmd, DEST_ADDR, FSP_PKT_WITH_ACK, &fsp_pkt);

    uint8_t encoded_frame[FSP_PKT_MAX_LENGTH];
    uint8_t frame_len;

    frame_encode(&fsp_pkt, encoded_frame, &frame_len);
    // BA
    /*
:  --> 00   -> PDU
:  --> 01   -> PMU (*)
:  --> 10   -> CAM
:  --> 11   -> IOU
     */
    LL_GPIO_ResetOutputPin(GPIOA, BOARD_SEL_B_Pin);
    LL_GPIO_SetOutputPin(GPIOA, BOARD_SEL_A_Pin);
    SCH_Delay(5);
    set_fsp_packet(encoded_frame, frame_len);
    set_send_flag();

    return CMDLINE_PENDING;
}

int Cmd_pmu_bat_vol(int argc, char *argv[])
{
    if (argc < 1) return CMDLINE_TOO_FEW_ARGS;
    if (argc > 1) return CMDLINE_TOO_MANY_ARGS;

    uint8_t cmd  = CMD_CODE_BAT_VOL;
    fsp_packet_t fsp_pkt;

    fsp_gen_cmd_pkt(cmd, DEST_ADDR, FSP_PKT_WITH_ACK, &fsp_pkt);

    uint8_t encoded_frame[FSP_PKT_MAX_LENGTH];
    uint8_t frame_len;

    frame_encode(&fsp_pkt, encoded_frame, &frame_len);
    // BA
    /*
:  --> 00   -> PDU
:  --> 01   -> PMU (*)
:  --> 10   -> CAM
:  --> 11   -> IOU
     */
    LL_GPIO_ResetOutputPin(GPIOA, BOARD_SEL_B_Pin);
    LL_GPIO_SetOutputPin(GPIOA, BOARD_SEL_A_Pin);
    SCH_Delay(5);
    set_fsp_packet(encoded_frame, frame_len);
    set_send_flag();

    return CMDLINE_PENDING;
}

int Cmd_pmu_parag_in(int argc, char *argv[])
{
    if (argc < 1) return CMDLINE_TOO_FEW_ARGS;
    if (argc > 1) return CMDLINE_TOO_MANY_ARGS;

    uint8_t cmd  = CMD_CODE_PARAG_IN;
    fsp_packet_t fsp_pkt;

    fsp_gen_cmd_pkt(cmd, DEST_ADDR, FSP_PKT_WITH_ACK, &fsp_pkt);

    uint8_t encoded_frame[FSP_PKT_MAX_LENGTH];
    uint8_t frame_len;

    frame_encode(&fsp_pkt, encoded_frame, &frame_len);
    // BA
    /*
:  --> 00   -> PDU
:  --> 01   -> PMU (*)
:  --> 10   -> CAM
:  --> 11   -> IOU
     */
    LL_GPIO_ResetOutputPin(GPIOA, BOARD_SEL_B_Pin);
    LL_GPIO_SetOutputPin(GPIOA, BOARD_SEL_A_Pin);
    SCH_Delay(5);
    set_fsp_packet(encoded_frame, frame_len);
    set_send_flag();

    return CMDLINE_PENDING;
}

int Cmd_pmu_parag_out(int argc, char *argv[])
{
    if (argc < 1) return CMDLINE_TOO_FEW_ARGS;
    if (argc > 1) return CMDLINE_TOO_MANY_ARGS;

    uint8_t cmd  = CMD_CODE_PARAG_OUT;
    fsp_packet_t fsp_pkt;

    fsp_gen_cmd_pkt(cmd, DEST_ADDR, FSP_PKT_WITH_ACK, &fsp_pkt);

    uint8_t encoded_frame[FSP_PKT_MAX_LENGTH];
    uint8_t frame_len;

    frame_encode(&fsp_pkt, encoded_frame, &frame_len);
    // BA
    /*
:  --> 00   -> PDU
:  --> 01   -> PMU (*)
:  --> 10   -> CAM
:  --> 11   -> IOU
     */
    LL_GPIO_ResetOutputPin(GPIOA, BOARD_SEL_B_Pin);
    LL_GPIO_SetOutputPin(GPIOA, BOARD_SEL_A_Pin);
    SCH_Delay(5);
    set_fsp_packet(encoded_frame, frame_len);
    set_send_flag();

    return CMDLINE_PENDING;
}

