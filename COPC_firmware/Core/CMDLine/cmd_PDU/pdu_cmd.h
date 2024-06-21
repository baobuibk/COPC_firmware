/*
 * pdu_cmd.h
 *
 *  Created on: Jun 10, 2024
 *      Author: CAO HIEU
 */

#ifndef CMDLINE_CMD_PDU_PDU_CMD_H_
#define CMDLINE_CMD_PDU_PDU_CMD_H_

#define CMD_CODE_PDU_SET_CHANNEL					0x01
#define CMD_CODE_PDU_SET_BUCK						0x02
#define CMD_CODE_PDU_SET_ALL						0x03
#define CMD_CODE_PDU_GET_CHANNEL					0x04
#define CMD_CODE_PDU_GET_BUCK						0x05
#define CMD_CODE_PDU_GET_ALL						0x06

int Cmd_pdu_set_channel(int argc, char *argv[]);
int Cmd_pdu_set_buck(int argc, char *argv[]);
int Cmd_pdu_set_all(int argc, char *argv[]);
int Cmd_pdu_get_channel(int argc, char *argv[]);
int Cmd_pdu_get_buck(int argc, char *argv[]);
int Cmd_pdu_get_all(int argc, char *argv[]);
void PDU_create_task(void);

#endif /* CMDLINE_CMD_PDU_PDU_CMD_H_ */
