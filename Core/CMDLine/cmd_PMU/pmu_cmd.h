/*
 * pmu_cmd.h
 *
 *  Created on: Jun 10, 2024
 *      Author: CAO HIEU
 */

#ifndef CMDLINE_CMD_PMU_PMU_CMD_H_
#define CMDLINE_CMD_PMU_PMU_CMD_H_


#define CMD_CODE_PMU_GET_TEMP						0x01
#define CMD_CODE_BAT_VOL							0x02
#define CMD_CODE_PARAG_IN							0x03
#define CMD_CODE_PARAG_OUT							0x04
#define CMD_CODE_PMU_TEMP_POINT						0x05
#define CMD_CODE_PMU_OUTPUT							0x06
#define CMD_CODE_PMU_PWM							0x07
#define CMD_CODE_PMU_ALL							0x08


int Cmd_pmu_get_temp(int argc, char *argv[]);
int Cmd_pmu_bat_vol(int argc, char *argv[]);
int Cmd_pmu_parag_in(int argc, char *argv[]);
int Cmd_pmu_parag_out(int argc, char *argv[]);
int Cmd_pmu_set_temppoint(int argc, char *argv[]);
int Cmd_pmu_set_output(int argc, char *argv[]);
int Cmd_pmu_set_pwm(int argc, char *argv[]);
int Cmd_pmu_get_all(int argc, char *argv[]);
void PMU_create_task(void);

#endif /* CMDLINE_CMD_PMU_PMU_CMD_H_ */
