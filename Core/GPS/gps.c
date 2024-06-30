/*
 * gps.c
 *
 *  Created on: Jun 20, 2024
 *      Author: nnmin
 */

#include "gps.h"
#include "../../Scheduler/scheduler.h"
#include <stm32f4xx_ll_usart.h>
#include "../../Core/CMDLine/global_vars.h"
#include <stdio.h>
#include "../../BSP/UART/uart.h"
#include <string.h>
#include <stdlib.h>

/*Private define*/
static void GPS_task_update(void);

/*Private typedef*/
typedef struct GPS_TaskContextTypedef
{
	SCH_TASK_HANDLE               taskHandle;
	SCH_TaskPropertyTypedef       taskProperty;
}GPS_TaskContextTypedef;

static GPS_TaskContextTypedef		gps_task_context =
{
		SCH_INVALID_TASK_HANDLE,                 // Will be updated by Schedular
		{
			SCH_TASK_SYNC,                      // taskType;
			SCH_TASK_PRIO_0,                    // taskPriority;
			1000,                               // taskPeriodInMS;
			GPS_task_update,					// taskFunction;
			985							//taskTick
		}

};


void GPS_create_task(void)
{
	Ringbuf_init();
	SCH_TASK_CreateTask(&gps_task_context.taskHandle, &gps_task_context.taskProperty);
}


uint16_t try_count = 0;
static void GPS_task_update(void)
{
    if (gps_report_enable) {
        uint8_t rxData;
        char gps_data[100] = {0};
        int index = 0;

        while (IsDataAvailable(USART3)) {
            rxData = Uart_read(USART3);

            if (!format_gps) {
                Uart_write(USART2, rxData);
                Uart_write(UART4, rxData);
            } else {
                gps_data[index++] = rxData;

                if (rxData == '\n') {
                    if (strstr(gps_data, "$GNGLL") != NULL) {
//                        char *token = strtok(gps_data, ",");
                        char *latitude = strtok(NULL, ",");
                        char *latitude_dir = strtok(NULL, ",");
                        char *longitude = strtok(NULL, ",");
                        char *longitude_dir = strtok(NULL, ",");
                        char *utc_time = strtok(NULL, ",");

                        if (strlen(latitude) == 0 && strlen(longitude) == 0) {
                            if (try_count < 3000) {
                                char msg[50];
                                sprintf(msg, "[Try: %d] GPS Getting data...\n", try_count);
                                Uart_sendstring(USART2, msg);
                                Uart_sendstring(UART4, msg);
                                try_count++;
                            } else {
                                try_count = 0;
                            }
                        } else if (strlen(latitude) == 0 && strlen(longitude) == 0 && strlen(utc_time) > 0) {
                            if (try_count < 3000) {
                                char formatted_utc[12];
                                sprintf(formatted_utc, "%.2s:%.2s:%.2s.%.2s", utc_time, utc_time + 2, utc_time + 4, utc_time + 7);
                                char msg[60];
                                sprintf(msg, "[Try: %d] UTC: %s, Try to get Position...\n", try_count, formatted_utc);
                                Uart_sendstring(USART2, msg);
                                Uart_sendstring(UART4, msg);
                                try_count++;
                            } else {
                                try_count = 0;
                            }
                        } else if (strlen(latitude) > 0 && strlen(longitude) > 0 && strlen(utc_time) > 0) {

                            char lat_str[20], lon_str[20];
                            double lat = atof(latitude);
                            double lon = atof(longitude);
                            int lat_deg = (int)(lat / 100);
                            double lat_min = lat - lat_deg * 100;
                            int lon_deg = (int)(lon / 100);
                            double lon_min = lon - lon_deg * 100;
                            sprintf(lat_str, "%d°%02d'%04.1f\"%s", lat_deg, (int)lat_min, (lat_min - (int)lat_min) * 60, latitude_dir);
                            sprintf(lon_str, "%d°%02d'%04.1f\"%s", lon_deg, (int)lon_min, (lon_min - (int)lon_min) * 60, longitude_dir);


                            char formatted_utc[12];
                            sprintf(formatted_utc, "%.2s:%.2s:%.2s.%.2s", utc_time, utc_time + 2, utc_time + 4, utc_time + 7);

                            char msg[100];
                            sprintf(msg, "[OK] UTC: %s, Position: %s %s\n", formatted_utc, lat_str, lon_str);
                            Uart_sendstring(USART2, msg);
                            Uart_sendstring(UART4, msg);
                            try_count = 0;
                        }
                    }

                    memset(gps_data, 0, sizeof(gps_data));
                    index = 0;
                }
            }
        }
    }
}
