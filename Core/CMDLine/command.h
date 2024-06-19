/*
 * command.h
 *
 *  Created on: May 23, 2024
 *      Author: CAO HIEU
 */

#ifndef CMDLINE_COMMAND_H_
#define CMDLINE_COMMAND_H_

#include "cmdline.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define	COMMAND_MAX_LENGTH	255

void	command_init(void);
void	command_create_task(void);
void	command_send_splash(void);


int Cmd_help(int argc, char *argv[]);
int Cmd_help_all(int argc, char *argv[]);
int Cmd_splash(int argc, char *argv[]);
int NotYetDefine(int argc, char *argv[]);
int Cmd_help_cpoc(int argc, char *argv[]);
int Cmd_help_pmu(int argc, char *argv[]);
int Cmd_help_pdu(int argc, char *argv[]);
int Cmd_help_cam(int argc, char *argv[]);
int Cmd_help_iou(int argc, char *argv[]);
int Cmd_status_now(int argc, char *argv[]);
int Cmd_memory_usage(int argc, char *argv[]);
int Cmd_auto_report_ena(int argc, char *argv[]);
//int Cmd_auto_report_dis(int argc, char *argv[]);
int Cmd_rs422_report_ena(int argc, char *argv[]);
int Cmd_set_byte_rs422(int argc, char *argv[]);
int Cmd_time_get(int argc, char *argv[]);
int Cmd_time_set(int argc, char *argv[]);
int Cmd_cpoc_reset(int argc, char *argv[]);
//int Cmd_board_alive(int argc, char *argv[]);
int Cmd_rf_ena(int argc, char *argv[]);
int Cmd_rf_dis(int argc, char *argv[]);


#endif /* CMDLINE_COMMAND_H_ */
