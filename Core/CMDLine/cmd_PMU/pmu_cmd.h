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


int Cmd_pmu_get_temp(int argc, char *argv[]);
int Cmd_pmu_bat_vol(int argc, char *argv[]);
int Cmd_pmu_parag_in(int argc, char *argv[]);
int Cmd_pmu_parag_out(int argc, char *argv[]);

#endif /* CMDLINE_CMD_PMU_PMU_CMD_H_ */
