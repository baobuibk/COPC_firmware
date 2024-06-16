/*
 * send_packet.h
 *
 *  Created on: May 28, 2024
 *      Author: CAO HIEU
 */

#ifndef CMDLINE_CMD_IOU_ACKSEND_PACKET_H_
#define CMDLINE_CMD_IOU_ACKSEND_PACKET_H_
#include "main.h"
#include "../../Scheduler/scheduler.h"

extern volatile uint8_t sendFlag;

void set_send_flag(void);
void clear_send_flag(void);
void send_packet_init(void);
void set_fsp_packet(uint8_t *encoded_pkt, uint8_t encoded_len);
void send_packet_create_task(void);



#endif /* CMDLINE_CMD_IOU_ACKSEND_PACKET_H_ */
