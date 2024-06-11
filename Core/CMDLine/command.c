/*
 * command.c
 *
 *  Created on: May 23, 2024
 *      Author: CAO HIEU
 */

#include "../../Scheduler/scheduler.h"
#include "command.h"
#include "../../BSP/UART/uart.h"
#include <stdlib.h>
#include "cmdline.h"
#include "stm32f4xx_ll_gpio.h"
#include "cmd_IOU/iou_cmd.h"
#include "cmd_PDU/pdu_cmd.h"
#include "cmd_PMU/pmu_cmd.h"
#include "cmd_CAM/cam_cmd.h"
#include "../../BSP/RTC/ds3231.h"
#include <stdio.h>
#include "main.h"



/* Private typedef -----------------------------------------------------------*/

typedef struct _Command_TaskContextTypedef_
{
	SCH_TASK_HANDLE               taskHandle;
	SCH_TaskPropertyTypedef       taskProperty;
} Command_TaskContextTypedef;

const char * ErrorCode[6] = {"OK\r\n", "CMDLINE_BAD_CMD\r\n", "CMDLINE_TOO_MANY_ARGS\r\n",
"CMDLINE_TOO_FEW_ARGS\r\n", "CMDLINE_INVALID_ARG\r\n", "CMD_OK_BUT_PENDING...\r\n" };

static	void	command_task_update(void);
tCmdLineEntry g_psCmdTable[] = {
								{"help", Cmd_help,": Display list of simple help commands | format: help" },
								{"help_all", Cmd_help_all,": Display list of ALL!!! commands | format: help_all" },
								{"help_cpoc", Cmd_help_cpoc,": Display list of CPOC commands | format: help_cpoc" },
								{"help_pmu", Cmd_help_pmu,": Display list of PMU commands | format: help_pmu" },
								{"help_pdu", Cmd_help_pdu,": Display list of PDU commands | format: help_pdu" },
								{"help_cam", Cmd_help_cam,": Display list of CAM commands | format: help_cam" },
								{"help_iou", Cmd_help_iou,": Display list of IOU commands | format: help_iou" },
								{"splash", Cmd_splash,": Splash screen again | format: splash" },
								{"status_now", Cmd_status_now,": Display <Date&Time>, <Temp>°C, <HardwareVer>, <FirmwareVer>, <Enable>, <Mode> | format: status_now" },
//CPOC
								{"time_get", Cmd_time_get , ": Get RTC Time | format: time_get"},
								{"time_set", Cmd_time_set , ": Time Setting | format: time_set <hh> <mm> <ss> <DD> <MM> <YY>"},
								{"cpoc_reset", Cmd_cpoc_reset , ": Reset CPOC Board | format: cpoc_reset", },
								{"board_alive", NotYetDefine, ": Hello to specified board, check alive | format: board_alive <board->(0: All, 1: COPC, 2: PMU, 3: PDU, 4: CAM, 5: IOU)"},
								{"mux_mode", NotYetDefine, ": Set Mux UART Mode | format: mux_mode <mode->(0: Disable, 1: Always, 2: Auto[Default])>"}	,
								{"rf_ena", Cmd_rf_ena, ": Enable RF Module | format: rf_ena"}	,
								{"rf_dis", Cmd_rf_dis, ": Disable RF Module | format: rf_dis"}	,
								{"lora_reset", NotYetDefine, ": Reset LORA Hardware |  format: reset_lora"}	,
								{"lora_status", NotYetDefine, ": Get Status LORA | format: status_lora"}	,
								{"lora_test", NotYetDefine, " : Send Message Test LORA | format: test_lora"}	,
								{"lora_set", NotYetDefine, ": Setting LORA Hardware |  format: ******** "}	,
								{"gps_reset", NotYetDefine, ": Reset GPS Hardware, format: gps_reset"}	,
								{"gps_status", NotYetDefine, ": Get Status GPS | format: gps_status"}	,
								{"gps_get", NotYetDefine, ": Get GPS Data | format: gps_get"}	,
								{"gps_set", NotYetDefine, ": Setting GPS Hardware | format: ********"}	,
								{"start_positioining", NotYetDefine, ": Continuously send  GPS Data to LORA | format: start_positioining"}	,
//PMU
								{"pmu_get_temp", Cmd_pmu_get_temp, ": Response 4 NTC channel in Celsius | format: pmu_temp"}	,
								{"pmu_bat_vol", Cmd_pmu_bat_vol, ": Response 4 BAT channel in Voltage | format: pmu_bat_vol"}	,
								{"pmu_parag_in", Cmd_pmu_parag_in, ": Response V_in, I_in from 28V source | format: pmu_parag_in"}	,
								{"pmu_parag_out", Cmd_pmu_parag_out, ": Response V_out, I_out from output 14.4V source | format: pmu_parag_out"}	,
//PDU
								{"pdu_set_channel", Cmd_pdu_set_channel, ": Turn on/off channel N | format: pdu_set_channel <channel> <state->(0: OFF, 1: ON)>"}	,
								{"pdu_set_buck", Cmd_pdu_set_buck, ": Turn on/off buck N | format: pdu_set_buck <buck> <state->(0: OFF, 1: ON)>"}	,
								{"pdu_set_all", Cmd_pdu_set_all, ": Turn on/off buck + channel | format: pdu_set_all <state->(0: OFF, 1: ON)>"}	,
								{"pdu_get_channel", Cmd_pdu_get_channel, ": Get parameter of channel N | format: pdu_get_channel <channel>"}	,
								{"pdu_get_buck", Cmd_pdu_get_buck, ": Get parameter of Buck N | format: pdu_get_buck <buck>"}	,
//CAM
								{"CAM_TEXT", NotYetDefine, ": CAM Blank | format: ********"}	,
//IOU
								{"iou_set_temp", Cmd_iou_set_temp, ": Set temperature of channel | format: iou_set_temp <channel> <temperature->(250 mean 25.0Cel)>"}	,
								{"iou_get_temp", Cmd_iou_get_temp, ": Response temperature of this channel | format: iou_get_temp <channel> <device->(0: NTC, 1: 1Wire)>"}	,
								{"iou_temp_setpoint", Cmd_iou_temp_setpoint, ": Response temperature set point of this channel | format: iou_temp_setpoint <channel>"}	,
								{"iou_tec_ena", Cmd_iou_tec_ena, ": Enable operation of this channel TEC | format: iou_tec_ena <channel>"}	,
								{"iou_tec_dis", Cmd_iou_tec_dis, ": Disable operation of this channel TEC | format: iou_tec_dis <channel>"}	,
								{"iou_tec_ena_auto", Cmd_iou_tec_ena_auto, ": Enable auto control this channel TEC | format: iou_tec_ena_auto <channel>"}	,
								{"iou_tec_dis_auto", Cmd_iou_tec_dis_auto, ": Disable auto control this channel TEC | format: iou_tec_dis_auto <channel>"}	,
								{"iou_tec_set_output", Cmd_iou_tec_set_output, ": Set output TEC Voltage  | format: iou_tec_set_output <channel> <mode->(0: Cool, 1: Heat)> <voltage->(150 mean 1.50)>"}	,
								{"iou_tec_auto_vol", Cmd_iou_tec_auto_vol, ": Automatically control TEC Voltage | format: iou_tec_set_auto_vol <channel> <voltage>"}	,
								{"iou_tec_status", Cmd_iou_tec_status, ": Get TEC status data | format: iou_tec_status"}	,
								{"iou_tec_log_ena", Cmd_iou_tec_log_ena, ": Enable periodic log | format: iou_tec_log_ena"}	,
								{"iou_tec_log_dis", Cmd_iou_tec_log_dis, ": Disable periodic log | format: iou_tec_log_dis"}	,
								{"iou_ringled_mode", Cmd_iou_ringled_mode, ": Set display mode for RingLed | format: iou_ringled_mode <mode->(0: blink_rgb, 1: blink_rainbow, 2: run_rainbow>"}	,
								{"iou_irled_set_bright", Cmd_iou_irled_set_bright, ": Set brightness (0-100%) of IR led | format: iou_irled_set_bright <percent->(0-100)>"}	,
								{"iou_irled_get_bright", Cmd_iou_irled_get_bright, ": Get brightness (0-100%) of IR led | format: iou_irled_get_bright "}	,
								{0,0,0}
								};
//
//set_temp
//get_temp
//get_temp_setpoint
//tec_ena
//tec_dis
//tec_ena_auto
//tec_dis_auto
//tec_set_output
//tec_set_auto_voltage
//tec_get_status
//tec_log_ena
//tec_log_dis
//ringled_set_mode
//ir_led_set_bright
//ir_led_get_bright



///*// Addresses
//#define FSP_ADR_COPC	                1       /**< COPC module address. */
//#define FSP_ADR_PMU                     2       /**< PMU module address. */
//#define FSP_ADR_PDU                     3       /**< PDU module address. */
//#define FSP_ADR_CAM	                    4       /**< CAM module address. */
//#define FSP_ADR_IOU		                5       /**< IOU module address. */*/

//volatile static	ring_buffer *p_CommandRingBuffer;
static	char s_commandBuffer[COMMAND_MAX_LENGTH];
static uint8_t	s_commandBufferIndex = 0;

static Command_TaskContextTypedef           s_CommandTaskContext =
{
	SCH_INVALID_TASK_HANDLE,                 // Will be updated by Schedular
	{
		SCH_TASK_SYNC,                      // taskType;
		SCH_TASK_PRIO_0,                    // taskPriority;
		10,                                // taskPeriodInMS;
		command_task_update                // taskFunction;
	}
};

void	command_init(void)
{
	Ringbuf_init();

//	p_CommandRingBuffer = uart_get_uart0_rx_buffer_address();
	memset((void *)s_commandBuffer, 0, sizeof(s_commandBuffer));
	s_commandBufferIndex = 0;
	Uart_sendstring(USART6,"\r\n");
	Uart_sendstring(USART6,"\r\n");
	Uart_sendstring(USART6,"> CPOC FIRMWARE V1.0.0 \r\n");
	Uart_sendstring(USART6,"\r\n");
	command_send_splash();

	tCmdLineEntry *pEntry;


	Uart_sendstring(USART6, "\nStart with <help_xxxx> command\r\n");
	Uart_sendstring(USART6, "---------------------------\r\n");
	pEntry = &g_psCmdTable[0];

	while (pEntry->pcCmd) {
		Uart_sendstring(USART6, pEntry->pcCmd);
		Uart_sendstring(USART6, pEntry->pcHelp);
		Uart_sendstring(USART6, "\r\n");
	    if (pEntry == &g_psCmdTable[8]) {
	        break;
	    }
	    pEntry++;
	}



	Uart_sendstring(USART6, "\r\n> ");


}

static void command_task_update(void)
{
    char rxData;
    int8_t ret_val;

    while (IsDataAvailable(USART6))
    {

        rxData = Uart_read(USART6);
        Uart_write(USART6, rxData);
        if ((rxData == '\r') || (rxData == '\n'))
        {
            if (s_commandBufferIndex > 0)
            {
                s_commandBuffer[s_commandBufferIndex] = 0;
                s_commandBufferIndex++;
                ret_val = CmdLineProcess(s_commandBuffer);
                s_commandBufferIndex = 0;
                Uart_sendstring(USART6, "\r\n> ");
                Uart_sendstring(USART6, ErrorCode[ret_val]);
                Uart_sendstring(USART6, "> ");
            }
            else
            {
                Uart_sendstring(USART6, "\r\n> ");
            }
        }
        else if ((rxData == 8) || (rxData == 127))
        {
            if (s_commandBufferIndex > 0)
            {
                s_commandBufferIndex--;
            }
        }
        else
        {
            s_commandBuffer[s_commandBufferIndex] = rxData;
            s_commandBufferIndex++;
            if (s_commandBufferIndex > COMMAND_MAX_LENGTH)
            {
                s_commandBufferIndex = 0;
            }
        }
    }
}


int Cmd_help(int argc, char *argv[]) {
	tCmdLineEntry *pEntry;
	Uart_sendstring(USART6, "\nSimple commands\r\n");
	Uart_sendstring(USART6, "------------------\r\n");
	pEntry = &g_psCmdTable[0];


	while (pEntry->pcCmd) {
		Uart_sendstring(USART6, pEntry->pcCmd);
		Uart_sendstring(USART6, pEntry->pcHelp);
		Uart_sendstring(USART6, "\r\n");
	    if (pEntry == &g_psCmdTable[8]) {
	        break;
	    }
	    pEntry++;
	}

	// Return success.
	return (CMDLINE_OK);

}


int Cmd_help_all(int argc, char *argv[]) {
	tCmdLineEntry *pEntry;

	Uart_sendstring(USART6, "\nAvailable commands\r\n");
	Uart_sendstring(USART6, "------------------\r\n");

	// Point at the beginning of the command table.
	pEntry = &g_psCmdTable[0];

	// Enter a loop to read each entry from the command table.  The
	// end of the table has been reached when the command name is NULL.
	while (pEntry->pcCmd) {
		// Print the command name and the brief description.
		Uart_sendstring(USART6, pEntry->pcCmd);
		Uart_sendstring(USART6, pEntry->pcHelp);
		Uart_sendstring(USART6, "\r\n");


	    if (pEntry == &g_psCmdTable[8]) {
	        Uart_sendstring(USART6, "\n---------CPOC Command List--------\r\n");
	    }

	    else if (pEntry == &g_psCmdTable[24]) {
	        Uart_sendstring(USART6, "\n---------PMU Command List--------\r\n");
	    }

	    else if (pEntry == &g_psCmdTable[28]) {
	        Uart_sendstring(USART6, "\n---------PDU Command List--------\r\n");
	    }

	    else if (pEntry == &g_psCmdTable[33]) {
	        Uart_sendstring(USART6, "\n---------CAM Command List--------\r\n");
	    }

	    else if (pEntry == &g_psCmdTable[34]) {
	        Uart_sendstring(USART6, "\n---------IOU Command List--------\r\n");
	    }


		// Advance to the next entry in the table.
		pEntry++;

	}
	Uart_sendstring(USART6, "---------    END    --------\r\n");
	// Return success.
	return (CMDLINE_OK);
}

int Cmd_help_cpoc(int argc, char *argv[]) {
	tCmdLineEntry *pEntry;
    Uart_sendstring(USART6, "---------CPOC Command List--------\r\n");
	// Point at the beginning of the command table.
	pEntry = &g_psCmdTable[9];

	// Enter a loop to read each entry from the command table.  The
	// end of the table has been reached when the command name is NULL.
	while (pEntry->pcCmd) {
		// Print the command name and the brief description.
		Uart_sendstring(USART6, pEntry->pcCmd);
		Uart_sendstring(USART6, pEntry->pcHelp);
		Uart_sendstring(USART6, "\r\n");



	    if (pEntry == &g_psCmdTable[24]) {
	        break;
	    }

		// Advance to the next entry in the table.
		pEntry++;

	}
	// Return success.
	return (CMDLINE_OK);

}
int Cmd_help_pmu(int argc, char *argv[]) {
	tCmdLineEntry *pEntry;
    Uart_sendstring(USART6, "---------PMU Command List--------\r\n");
	// Point at the beginning of the command table.
	pEntry = &g_psCmdTable[25];

	// Enter a loop to read each entry from the command table.  The
	// end of the table has been reached when the command name is NULL.
	while (pEntry->pcCmd) {
		// Print the command name and the brief description.
		Uart_sendstring(USART6, pEntry->pcCmd);
		Uart_sendstring(USART6, pEntry->pcHelp);
		Uart_sendstring(USART6, "\r\n");



	    if (pEntry == &g_psCmdTable[28]) {
	        break;
	    }

		// Advance to the next entry in the table.
		pEntry++;

	}
	// Return success.
	return (CMDLINE_OK);
}
int Cmd_help_pdu(int argc, char *argv[]) {
	tCmdLineEntry *pEntry;
    Uart_sendstring(USART6, "---------PDU Command List--------\r\n");

	// Point at the beginning of the command table.
	pEntry = &g_psCmdTable[29];

	// Enter a loop to read each entry from the command table.  The
	// end of the table has been reached when the command name is NULL.
	while (pEntry->pcCmd) {
		// Print the command name and the brief description.
		Uart_sendstring(USART6, pEntry->pcCmd);
		Uart_sendstring(USART6, pEntry->pcHelp);
		Uart_sendstring(USART6, "\r\n");



	    if (pEntry == &g_psCmdTable[33]) {
	        break;
	    }

		// Advance to the next entry in the table.
		pEntry++;

	}
	// Return success.
	return (CMDLINE_OK);
}
int Cmd_help_cam(int argc, char *argv[]) {
	tCmdLineEntry *pEntry;
    Uart_sendstring(USART6, "---------CAM Command List--------\r\n");

	// Point at the beginning of the command table.
	pEntry = &g_psCmdTable[34];

	// Enter a loop to read each entry from the command table.  The
	// end of the table has been reached when the command name is NULL.
	while (pEntry->pcCmd) {
		// Print the command name and the brief description.
		Uart_sendstring(USART6, pEntry->pcCmd);
		Uart_sendstring(USART6, pEntry->pcHelp);
		Uart_sendstring(USART6, "\r\n");



	    if (pEntry == &g_psCmdTable[34]) {
	        break;
	    }

		// Advance to the next entry in the table.
		pEntry++;

	}
	// Return success.
	return (CMDLINE_OK);
}
int Cmd_help_iou(int argc, char *argv[]) {
	tCmdLineEntry *pEntry;
    Uart_sendstring(USART6, "---------IOU Command List--------\r\n");


	// Point at the beginning of the command table.
	pEntry = &g_psCmdTable[35];

	// Enter a loop to read each entry from the command table.  The
	// end of the table has been reached when the command name is NULL.
	while (pEntry->pcCmd) {
		// Print the command name and the brief description.
		Uart_sendstring(USART6, pEntry->pcCmd);
		Uart_sendstring(USART6, pEntry->pcHelp);
		Uart_sendstring(USART6, "\r\n");


	    if (pEntry == &g_psCmdTable[23]) {
	        break;
	    }

		// Advance to the next entry in the table.
		pEntry++;

	}
	// Return success.
	return (CMDLINE_OK);
}





int NotYetDefine(int argc, char *argv[]) {
	Uart_sendstring(USART6, "\nThis function is not defined yet \r\n");
	// Return success.
	return (CMDLINE_OK);
}

int Cmd_splash(int argc, char *argv[]) {
	command_send_splash();
	// Return success.
	return (CMDLINE_OK);
}

int Cmd_status_now(int argc, char *argv[]){
    uint8_t day, date, month, year, hour, min, sec;
    float temp;
    char buffer[100];

    // Get current date and time from DS3231
    DS3231_GetDateTime(&day, &date, &month, &year, &hour, &min, &sec);

    // Get current temperature from DS3231
    temp = DS3231_GetTemperature();

    sprintf(buffer, "%02d:%02d:%02d %02d/%02d/%04d\r\n", hour, min, sec, date, month, 2000 + year);
    Uart_sendstring(USART6, buffer);

    sprintf(buffer, "Temperature: %.2f°C\r\n", temp);
    Uart_sendstring(USART6, buffer);
    sprintf(buffer, "HardwareVer: CPOC Hardware 1.2.0\r\n");
    Uart_sendstring(USART6, buffer);
    sprintf(buffer, "FirmwareVer: CPOC Firmware 1.0.0\r\n");
    Uart_sendstring(USART6, buffer);

    sprintf(buffer, "Enable: Yes\r\n");
    Uart_sendstring(USART6, buffer);

    sprintf(buffer, "Mode: Normal\r\n");
    Uart_sendstring(USART6, buffer);

	// Return success.
	return (CMDLINE_OK);
}

int Cmd_time_get(int argc, char *argv[]){
    uint8_t day, date, month, year, hour, min, sec;
    char buffer[100];

    // Get current date and time from DS3231
    DS3231_GetDateTime(&day, &date, &month, &year, &hour, &min, &sec);
    sprintf(buffer, "%02d:%02d:%02d %02d/%02d/%04d\r\n", hour, min, sec, date, month, 2000 + year);    Uart_sendstring(USART6, buffer);
	// Return success.
	return (CMDLINE_OK);
}

int Cmd_time_set(int argc, char *argv[]){
    if (argc < 7) return CMDLINE_TOO_FEW_ARGS;
    if (argc > 7) return CMDLINE_TOO_MANY_ARGS;

    uint8_t hour = atoi(argv[1]);
    uint8_t min = atoi(argv[2]);
    uint8_t sec = atoi(argv[3]);
    uint8_t date = atoi(argv[4]);
    uint8_t month = atoi(argv[5]);
    uint8_t year = atoi(argv[6]);

    if (hour > 23 || min > 59 || sec > 59 || date > 31 || month > 12 || year > 99)
        return CMDLINE_INVALID_ARG;

    DS3231_SetDateTime(1, date, month, year, hour, min, sec);

    char buffer[100];
    sprintf(buffer, "Time set to: %02d:%02d:%02d %02d/%02d/%04d\r\n", hour, min, sec, date, month, 2000 + year);
    Uart_sendstring(USART6, buffer);

	// Return success.
	return (CMDLINE_OK);
}

int Cmd_cpoc_reset(int argc, char *argv[]){
	NVIC_SystemReset();
	// Return success.
	return (CMDLINE_OK);
}

int Cmd_rf_ena(int argc, char *argv[]){
	LL_GPIO_ResetOutputPin(ENABLE_RF_GPIO_Port, ENABLE_RF_Pin);
	 Uart_sendstring(USART6, "\nRF Set to Enable\r\n");
	// Return success.
	return (CMDLINE_OK);
}

int Cmd_rf_dis(int argc, char *argv[]){
	LL_GPIO_SetOutputPin(ENABLE_RF_GPIO_Port, ENABLE_RF_Pin);
	Uart_sendstring(USART6, "\nRF Disable\r\n");
	// Return success.
	return (CMDLINE_OK);
}




void	command_create_task(void)
{
	SCH_TASK_CreateTask(&s_CommandTaskContext.taskHandle, &s_CommandTaskContext.taskProperty);
}

void	command_send_splash(void)
{
	Uart_sendstring(USART6, "\r\n");

	Uart_sendstring(USART6, "................................................\r\n");
	Uart_sendstring(USART6, "..        ____                                ..\r\n");
	Uart_sendstring(USART6, "..       / ___| _ __   __ _  ___ ___          ..\r\n");
	Uart_sendstring(USART6, "..       \\___ \\| '_ \\ / _` |/ __/ _ \\         ..  \r\n");
	Uart_sendstring(USART6, "..        ___) | |_) | (_| | (_|  __/         ..\r\n");
	Uart_sendstring(USART6, "..       |____/| .__/ \\__,_|\\___\\___|         ..  \r\n");
	Uart_sendstring(USART6, "..             |_|                            ..\r\n");
	Uart_sendstring(USART6, "..     _     _ _     _____         _          ..\r\n");
	Uart_sendstring(USART6, "..    | |   (_|_)_ _|_   _|__  ___| |__       ..\r\n");
	Uart_sendstring(USART6, "..    | |   | | | '_ \\| |/ _ \\/ __| '_ \\      ..\r\n");
	Uart_sendstring(USART6, "..    | |___| | | | | | |  __/ (__| | | |     ..\r\n");
	Uart_sendstring(USART6, "..    |_____|_|_|_| |_|_|\\___|\\___| |_|_|     ..\r\n");
	Uart_sendstring(USART6, "................................................\r\n");
	Uart_sendstring(USART6, "..           ____ ____   ___   ____           ..\r\n");
	Uart_sendstring(USART6, "..          / ___|  _ \\ / _ \\ / ___|          ..\r\n");
	Uart_sendstring(USART6, "..         | |   | |_) | | | | |              ..\r\n");
	Uart_sendstring(USART6, "..         | |___|  __/| |_| | |___           ..\r\n");
	Uart_sendstring(USART6, "..          \\____|_|    \\___/ \\____|          .. \r\n");
	Uart_sendstring(USART6, "..                  _   ___   ___             ..\r\n");
	Uart_sendstring(USART6, "..          __   __/ | / _ \\ / _ \\            .. \r\n");
	Uart_sendstring(USART6, "..          \\ \\ / /| || | | | | | |           ..  \r\n");
	Uart_sendstring(USART6, "..           \\ V / | || |_| | |_| |           .. \r\n");
	Uart_sendstring(USART6, "..            \\_/  |_(_)___(_)___/            .. \r\n");
    Uart_sendstring(USART6, "................................................\r\n");

	Uart_sendstring(USART6, "> ");
}
