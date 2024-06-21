/*
 * CPOC.h
 *
 *  Created on: May 25, 2024
 *      Author: CAO HIEU
 */

#ifndef CPOC_CPOC_H_
#define CPOC_CPOC_H_


#include "../../BSP/UART/uart.h"
typedef struct _COMMON_FRAME_
{
	uint8_t Cmd;
}COMMON_FRAME;
typedef struct _PDU_SINGLE_POWER_CONTROL_FRAME_
{
	uint8_t Cmd;              /* The command class */
	uint8_t PowerSource : 7;                   /* power source */
	uint8_t Status		:1;						/* 1: ON, 0: OFF */
} PDU_SINGLE_POWER_CONTROL_FRAME;

typedef struct _PDU_SINGLE_POWER_STATUS_REQUEST_FRAME_
{
	uint8_t Cmd;              /* The command class */
	uint8_t PowerSource;                   /* power source */

} PDU_SINGLE_POWER_STATUS_REQUEST_FRAME;

typedef struct _PDU_SINGLE_POWER_STATUS_REPORT_FRAME_
{
	uint8_t		Cmd;								/* The command class */
	uint8_t		PowerSource : 7;                   /* power source */
	uint8_t		Status		:1;						/* 1: ON, 0: OFF */
	uint16_t	Current_Val;
}	PDU_SINGLE_POWER_STATUS_REPORT_FRAME;

// Union to encapsulate all frame types

typedef union _COPC_Sfp_Payload_ {
	COMMON_FRAME							commonFrame;
	PDU_SINGLE_POWER_CONTROL_FRAME			powerControlFrame;
	PDU_SINGLE_POWER_STATUS_REQUEST_FRAME	powerStatusRequestFrame;
	PDU_SINGLE_POWER_STATUS_REPORT_FRAME	powerStatusReportFrame;
} COPC_Sfp_Payload_t;
void COPC_init(void);

void	COPC_create_task(void);
#endif /* CPOC_CPOC_H_ */
