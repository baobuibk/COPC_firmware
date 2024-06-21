/*
 * rs422.c
 *
 *  Created on: Jun 16, 2024
 *      Author: CAO HIEU
 */

//#define CMD_CODE_PDU_GET_ALL		0x06
//#define CMD_CODE_PMU_GET_ALL		0x08
//#define CMD_CODE_CAM_GET_ALL		0x0B
//#define CMD_CODE_IOU_GET_ALL		0x13
#include "rs422.h"


#include <string.h>
#include "../../BSP/UART/uart.h"
#include "global_vars.h"
#include "main.h"
#include "../../Scheduler/scheduler.h"
#include <stdio.h>
#include <stdlib.h>
#include "../../ThirdParty/libfsp/crc.h"
#include "../../BSP/RTC/ds3231.h"

#define RS422_PERIOD 1000

static void RS422_periodic_task(void);


typedef struct RS422_TaskContextTypedef
{
	SCH_TASK_HANDLE               taskHandle;
	SCH_TaskPropertyTypedef       taskProperty;
} RS422_TaskContextTypedef;


static RS422_TaskContextTypedef           RS422_task_context =
{
	SCH_INVALID_TASK_HANDLE,                 // Will be updated by Schedular
	{
		SCH_TASK_SYNC,                      // taskType;
		SCH_TASK_PRIO_0,                    // taskPriority;
		100,                                // taskPeriodInMS;
		RS422_periodic_task					// taskFunction;
	}
};



void switch_board(uint8_t board_id) {
    switch (board_id) {
        case 0: // PDU
            LL_GPIO_ResetOutputPin(GPIOA, BOARD_SEL_B_Pin);
            LL_GPIO_ResetOutputPin(GPIOA, BOARD_SEL_A_Pin);
            break;
        case 1: // PMU
            LL_GPIO_ResetOutputPin(GPIOA, BOARD_SEL_B_Pin);
            LL_GPIO_SetOutputPin(GPIOA, BOARD_SEL_A_Pin);
            break;
        case 2: // CAM
            LL_GPIO_SetOutputPin(GPIOA, BOARD_SEL_B_Pin);
            LL_GPIO_ResetOutputPin(GPIOA, BOARD_SEL_A_Pin);
            break;
        case 3: // IOU
            LL_GPIO_SetOutputPin(GPIOA, BOARD_SEL_B_Pin);
            LL_GPIO_SetOutputPin(GPIOA, BOARD_SEL_A_Pin);
            break;
    }
}

/*
 * PDU: CA 01 03 01 04 06 AA BF EF
 * PMU: CA 01 02 01 04 08 3D C5 EF
 * IOU: CA 01 05 01 04 13 CF B2 EF
 */

//#define FSP_ADR_CPOC	                1       /**< CPOC module address. */
//#define FSP_ADR_PMU                     2       /**< PMU module address. */
//#define FSP_ADR_PDU                     3       /**< PDU module address. */
//#define FSP_ADR_CAM	                    4       /**< CAM module address. */
//#define FSP_ADR_IOU		                5       /**< IOU module address. */



uint8_t sourceArray[1000];
uint8_t destArray[1000];

volatile uint8_t count_packet = 0;
void RS422_periodic_task(void) {
	if (rs422_report_enable) {
		if (SCH_TIM_HasCompleted(SCH_TIM_RS422))
		{

			sourceArray[0] = 0x02;
			sourceArray[ARRAY_SIZE - 1] = 0x03;

		    uint8_t day, date, month, year, hour, min, sec;
		    DS3231_GetDateTime(&day, &date, &month, &year, &hour, &min, &sec);

		    float temp;
		    uint8_t rounded_temp;

		    temp = DS3231_GetTemperature();
		    rounded_temp = (uint8_t)temp;

		    sourceArray[2] = sec;
		    sourceArray[3] = min;
		    sourceArray[4] = hour;
		    sourceArray[5] = date;
		    sourceArray[6] = month;
		    sourceArray[7] = rounded_temp;


			count_packet = 0;
			for (int i = 121; i <= 146; i++) {
			    sourceArray[i] = i - 121;
			}


			for (int i = 147; i <= ARRAY_SIZE-2; i++) {
			    sourceArray[i] = i - 147;
			}


			uint16_t crc = crc16_CCITT(0xFFFF, &sourceArray[1], ARRAY_SIZE - 4);

			sourceArray[ARRAY_SIZE - 3] = (crc >> 8) & 0xFF;  // CRC#HIGH
			sourceArray[ARRAY_SIZE - 2] = crc & 0xFF;         // CRC#LOW


			memcpy(destArray, sourceArray, sizeof(sourceArray));
			SCH_TIM_Start(SCH_TIM_RS422, RS422_PERIOD);
		}
		sourceArray[1] = count_packet++;

        for (int i = 0; i < ARRAY_SIZE; i++) {
            Uart_write(UART5, destArray[i]);
        }
	}
}

void frame_processing_rs422(fsp_packet_t *fsp_pkt){
	switch(fsp_pkt->payload[0])
	{
		case 0x08:
	    {
			if(!rs422_report_enable){
				Uart_sendstring(UART5, "\nPMU:\n");
				int16_t ntc0 = (int16_t)((fsp_pkt->payload[1] << 8) | fsp_pkt->payload[2]);
				int16_t ntc1 = (int16_t)((fsp_pkt->payload[3] << 8) | fsp_pkt->payload[4]);
				int16_t ntc2 = (int16_t)((fsp_pkt->payload[5] << 8) | fsp_pkt->payload[6]);
				int16_t ntc3 = (int16_t)((fsp_pkt->payload[7] << 8) | fsp_pkt->payload[8]);

				uint16_t bat0 = (uint16_t)((fsp_pkt->payload[9] << 8) | fsp_pkt->payload[10]);
				uint16_t bat1 = (uint16_t)((fsp_pkt->payload[11] << 8) | fsp_pkt->payload[12]);
				uint16_t bat2 = (uint16_t)((fsp_pkt->payload[13] << 8) | fsp_pkt->payload[14]);
				uint16_t bat3 = (uint16_t)((fsp_pkt->payload[15] << 8) | fsp_pkt->payload[16]);

				uint16_t vin = (uint16_t)((fsp_pkt->payload[17] << 8) | fsp_pkt->payload[18]);
				uint16_t iin = (uint16_t)((fsp_pkt->payload[19] << 8) | fsp_pkt->payload[20]);

				uint16_t vout = (uint16_t)((fsp_pkt->payload[21] << 8) | fsp_pkt->payload[22]);
				uint16_t iout = (uint16_t)((fsp_pkt->payload[23] << 8) | fsp_pkt->payload[24]);

				char buffer_0x08[500];
				sprintf(buffer_0x08, "PMU_Res: CMDcode 0x08 [\nNTC0: %s%d.%02d, \nNTC1: %s%d.%02d, \nNTC2: %s%d.%02d, \nNTC3: %s%d.%02d, \nBAT0: %d.%02d V, \nBAT1: %d.%02d V, \nBAT2: %d.%02d V, \nBAT3: %d.%02d V, \nVIN: %d.%02d V, \nIIN: %d.%02d A, \nVOUT: %d.%02d V, \nIOUT: %d.%02d A]\n",
						ntc0 < 0 ? "-" : "", abs(ntc0) / 100, abs(ntc0) % 100,
						ntc1 < 0 ? "-" : "", abs(ntc1) / 100, abs(ntc1) % 100,
						ntc2 < 0 ? "-" : "", abs(ntc2) / 100, abs(ntc2) % 100,
						ntc3 < 0 ? "-" : "", abs(ntc3) / 100, abs(ntc3) % 100,
						bat0 / 100, bat0 % 100, bat1 / 100, bat1 % 100,
						bat2 / 100, bat2 % 100, bat3 / 100, bat3 % 100,
						vin / 100, vin % 100, iin / 100, iin % 100,
						vout / 100, vout % 100, iout / 100, iout % 100);
				Uart_sendstring(UART5, buffer_0x08);
			}
			receive_pmuFlag = 1;

			for (int i = 1; i <= 24; i++) {
			    sourceArray[i + 96] = fsp_pkt->payload[i]; //97   pay1    + 98 pay2    120    pay24
			}
			disconnect_counter_pmu = 0;

	    }
	    break;

		case 0x06:
		{
			if(!rs422_report_enable){
				Uart_sendstring(UART5, "\nPDU:\n");
				uint8_t tec1buck_status = fsp_pkt->payload[1];
				uint16_t tec1buck_voltage = (fsp_pkt->payload[2] << 8) | fsp_pkt->payload[3];

				uint8_t tec2buck_status = fsp_pkt->payload[4];
				uint16_t tec2buck_voltage = (fsp_pkt->payload[5] << 8) | fsp_pkt->payload[6];

				uint8_t tec3buck_status = fsp_pkt->payload[7];
				uint16_t tec3buck_voltage = (fsp_pkt->payload[8] << 8) | fsp_pkt->payload[9];

				uint8_t tec4buck_status = fsp_pkt->payload[10];
				uint16_t tec4buck_voltage = (fsp_pkt->payload[11] << 8) | fsp_pkt->payload[12];

				uint8_t mcubuck_status = fsp_pkt->payload[13];
				uint16_t mcubuck_voltage = (fsp_pkt->payload[14] << 8) | fsp_pkt->payload[15];

				uint8_t ledbuck_status = fsp_pkt->payload[16];
				uint16_t ledbuck_voltage = (fsp_pkt->payload[17] << 8) | fsp_pkt->payload[18];

				uint8_t cm4buck_status = fsp_pkt->payload[19];
				uint16_t cm4buck_voltage = (fsp_pkt->payload[20] << 8) | fsp_pkt->payload[21];

				uint8_t tec1_status = fsp_pkt->payload[22];
				uint16_t tec1_current = (fsp_pkt->payload[23] << 8) | fsp_pkt->payload[24];

				uint8_t tec2_status = fsp_pkt->payload[25];
				uint16_t tec2_current = (fsp_pkt->payload[26] << 8) | fsp_pkt->payload[27];

				uint8_t tec3_status = fsp_pkt->payload[28];
				uint16_t tec3_current = (fsp_pkt->payload[29] << 8) | fsp_pkt->payload[30];

				uint8_t tec4_status = fsp_pkt->payload[31];
				uint16_t tec4_current = (fsp_pkt->payload[32] << 8) | fsp_pkt->payload[33];

				uint8_t copc_status = fsp_pkt->payload[34];
				uint16_t copc_current = (fsp_pkt->payload[35] << 8) | fsp_pkt->payload[36];

				uint8_t iou_status = fsp_pkt->payload[37];
				uint16_t iou_current = (fsp_pkt->payload[38] << 8) | fsp_pkt->payload[39];

				uint8_t rgb_status = fsp_pkt->payload[40];
				uint16_t rgb_current = (fsp_pkt->payload[41] << 8) | fsp_pkt->payload[42];

				uint8_t ir_status = fsp_pkt->payload[43];
				uint16_t ir_current = (fsp_pkt->payload[44] << 8) | fsp_pkt->payload[45];

				uint8_t cm4_status = fsp_pkt->payload[46];
				uint16_t cm4_current = (fsp_pkt->payload[47] << 8) | fsp_pkt->payload[48];

				uint8_t vin_status = fsp_pkt->payload[49];
				uint16_t vin_voltage = (fsp_pkt->payload[50] << 8) | fsp_pkt->payload[51];

				uint8_t vbus_status = fsp_pkt->payload[52];
				uint16_t vbus_voltage = (fsp_pkt->payload[53] << 8) | fsp_pkt->payload[54];


						char buffer_0x06[1000];
						sprintf(buffer_0x06, "PDU_Res: CMDcode 0x06 [TEC1BUCK: Status %u, Voltage: %u\r\nTEC2BUCK: Status %u, Voltage: %u\r\nTEC3BUCK: Status %u, Voltage: %u\r\nTEC4BUCK: Status %u, Voltage: %u\r\nMCUBUCK: Status %u, Voltage: %u\r\nLEDBUCK: Status %u, Voltage: %u\r\nCM4BUCK: Status %u, Voltage: %u\r\nTEC1: Status %u, Current: %u\r\nTEC2: Status %u, Current: %u\r\nTEC3: Status %u, Current: %u\r\nTEC4: Status %u, Current: %u\r\nCOPC: Status %u, Current: %u\r\nIOU: Status %u, Current: %u\r\nRGB: Status %u, Current: %u\r\nIR: Status %u, Current: %u\r\nCM4: Status %u, Current: %u\r\nVIN: Status %u, Voltage: %u\r\nVBUS: Status %u, Voltage: %u\r\n]",
							tec1buck_status, tec1buck_voltage,
							tec2buck_status, tec2buck_voltage,
							tec3buck_status, tec3buck_voltage,
							tec4buck_status, tec4buck_voltage,
							mcubuck_status, mcubuck_voltage,
							ledbuck_status, ledbuck_voltage,
							cm4buck_status, cm4buck_voltage,
							tec1_status, tec1_current,
							tec2_status, tec2_current,
							tec3_status, tec3_current,
							tec4_status, tec4_current,
							copc_status, copc_current,
							iou_status, iou_current,
							rgb_status, rgb_current,
							ir_status, ir_current,
							cm4_status, cm4_current,
							vin_status, vin_voltage,
							vbus_status, vbus_voltage);

						Uart_sendstring(UART5, buffer_0x06);
			}

					receive_pduFlag = 1;

					for (int i = 1; i <= 54; i++) {
					    sourceArray[i + 42] = fsp_pkt->payload[i]; //43   pay1    + 44  pay2        96-<54
					}
					disconnect_counter_pdu = 0;
		}
		break;


		case 0x13:
		{
			if(!rs422_report_enable){
			Uart_sendstring(UART5, "\nIOU:\n");
			int16_t temp_ntc_channel0 = (int16_t)((fsp_pkt->payload[1] << 8) | fsp_pkt->payload[2]);
			int16_t temp_ntc_channel1 = (int16_t)((fsp_pkt->payload[3] << 8) | fsp_pkt->payload[4]);
			int16_t temp_ntc_channel2 = (int16_t)((fsp_pkt->payload[5] << 8) | fsp_pkt->payload[6]);
			int16_t temp_ntc_channel3 = (int16_t)((fsp_pkt->payload[7] << 8) | fsp_pkt->payload[8]);

			int16_t temp_onewire_channel0 = (int16_t)((fsp_pkt->payload[9] << 8) | fsp_pkt->payload[10]);
			int16_t temp_onewire_channel1 = (int16_t)((fsp_pkt->payload[11] << 8) | fsp_pkt->payload[12]);

			int16_t temp_sensor = (int16_t)((fsp_pkt->payload[13] << 8) | fsp_pkt->payload[14]);

			int16_t temp_setpoint_channel0 = (int16_t)((fsp_pkt->payload[15] << 8) | fsp_pkt->payload[16]);
			int16_t temp_setpoint_channel1 = (int16_t)((fsp_pkt->payload[17] << 8) | fsp_pkt->payload[18]);
			int16_t temp_setpoint_channel2 = (int16_t)((fsp_pkt->payload[19] << 8) | fsp_pkt->payload[20]);
			int16_t temp_setpoint_channel3 = (int16_t)((fsp_pkt->payload[21] << 8) | fsp_pkt->payload[22]);

			uint16_t voltage_out_tec_channel0 = (uint16_t)((fsp_pkt->payload[23] << 8) | fsp_pkt->payload[24]);
			uint16_t voltage_out_tec_channel1 = (uint16_t)((fsp_pkt->payload[25] << 8) | fsp_pkt->payload[26]);
			uint16_t voltage_out_tec_channel2 = (uint16_t)((fsp_pkt->payload[27] << 8) | fsp_pkt->payload[28]);
			uint16_t voltage_out_tec_channel3 = (uint16_t)((fsp_pkt->payload[29] << 8) | fsp_pkt->payload[30]);

			uint8_t neo_led_r = fsp_pkt->payload[31];
			uint8_t neo_led_g = fsp_pkt->payload[32];
			uint8_t neo_led_b = fsp_pkt->payload[33];
			uint8_t neo_led_w = fsp_pkt->payload[34];

			uint8_t ir_led_duty = fsp_pkt->payload[35];

			char buffer_0x13[1000];
			sprintf(buffer_0x13, "IOU_Res: CMDcode 0x13 [NTC Temp: Ch0=%s%d.%d, Ch1=%s%d.%d, Ch2=%s%d.%d, Ch3=%s%d.%d\n"
			                     "OneWire Temp: Ch0=%s%d.%d, Ch1=%s%d.%d\n"
			                     "Sensor Temp: %s%d.%d\n"
			                     "Setpoint Temp: Ch0=%s%d.%d, Ch1=%s%d.%d, Ch2=%s%d.%d, Ch3=%s%d.%d\n"
			                     "TEC Voltage: Ch0=%d.%02d, Ch1=%d.%02d, Ch2=%d.%02d, Ch3=%d.%02d\n"
			                     "Neo LED: R=%u, G=%u, B=%u, W=%u\n"
			                     "IR LED Duty: %u%%]\n",
			        temp_ntc_channel0 < 0 ? "-" : "", abs(temp_ntc_channel0)/ 10, abs(temp_ntc_channel0) % 10,
			        temp_ntc_channel1 < 0 ? "-" : "", abs(temp_ntc_channel1)/ 10, abs(temp_ntc_channel1) % 10,
			        temp_ntc_channel2 < 0 ? "-" : "", abs(temp_ntc_channel2)/ 10, abs(temp_ntc_channel2) % 10,
			        temp_ntc_channel3 < 0 ? "-" : "", abs(temp_ntc_channel3)/ 10, abs(temp_ntc_channel3) % 10,
			        temp_onewire_channel0 < 0 ? "-" : "", abs(temp_onewire_channel0)/ 10, abs(temp_onewire_channel0) % 10,
			        temp_onewire_channel1 < 0 ? "-" : "", abs(temp_onewire_channel1)/ 10, abs(temp_onewire_channel1) % 10,
			        temp_sensor < 0 ? "-" : "", abs(temp_sensor)/ 10, abs(temp_sensor) % 10,
			        temp_setpoint_channel0 < 0 ? "-" : "", abs(temp_setpoint_channel0)/ 10, abs(temp_setpoint_channel0) % 10,
			        temp_setpoint_channel1 < 0 ? "-" : "", abs(temp_setpoint_channel1)/ 10, abs(temp_setpoint_channel1) % 10,
			        temp_setpoint_channel2 < 0 ? "-" : "", abs(temp_setpoint_channel2)/ 10, abs(temp_setpoint_channel2) % 10,
			        temp_setpoint_channel3 < 0 ? "-" : "", abs(temp_setpoint_channel3)/ 10, abs(temp_setpoint_channel3) % 10,
			        voltage_out_tec_channel0 / 100, voltage_out_tec_channel0 % 100,
			        voltage_out_tec_channel1 / 100, voltage_out_tec_channel1 % 100,
			        voltage_out_tec_channel2 / 100, voltage_out_tec_channel2 % 100,
			        voltage_out_tec_channel3 / 100, voltage_out_tec_channel3 % 100,
			        neo_led_r, neo_led_g, neo_led_b, neo_led_w,
			        ir_led_duty);

			Uart_sendstring(UART5, buffer_0x13);
			}
			receive_iouFlag = 1;


			for (int i = 1; i <= 35; i++) {
					    sourceArray[i + 7] = fsp_pkt->payload[i]; //42   =  35  + 7      8 -> pay 1   9 -> pay2    43 -< pay35
			}

			disconnect_counter_iou = 0;

		}
		break;


		default:
			Uart_sendstring(USART6, "Failed to get all");
			break;
	}

}

void	rs422_create_task(void)
{
	SCH_TASK_CreateTask(&RS422_task_context.taskHandle, &RS422_task_context.taskProperty);
	Ringbuf_init();
}




//dau tien la 2s 1lan, sau do la  1s 1 lan

