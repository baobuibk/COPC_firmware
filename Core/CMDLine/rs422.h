/*
 * rs422.h
 *
 *  Created on: Jun 16, 2024
 *      Author: CAO HIEU
 */

#ifndef CMDLINE_RS422_H_
#define CMDLINE_RS422_H_
#include "../../ThirdParty/libfsp/fsp.h"
void switch_board(uint8_t board_id);
void frame_processing_rs422(fsp_packet_t *fsp_pkt);
void rs422_create_task(void);
void rs422_set_task_period(uint32_t period_ms);
int Cmd_send_frame_status(int argc, char *argv[]);
int Cmd_send_frame_cam(int argc, char *argv[]);
#endif /* CMDLINE_RS422_H_ */
