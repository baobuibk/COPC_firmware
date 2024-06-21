/*
 * sys.h
 *
 *  Created on: May 23, 2024
 *      Author: CAO HIEU
 */

#ifndef SYS_H_
#define SYS_H_

#include "../Common/basetypedef.h"

typedef enum SYS_EventTypeDef
{
	eSYS_EVENT_First = 0,
	eSYS_EVENT_LostMainPower = eSYS_EVENT_First,
	eSYS_EVENT_INT_I2C,
	eSYS_EVENT_MAX
}SYS_EventTypeDef;

typedef	enum SYS_SystemStateTypeDef
{
	eSYS_STATE_First = 0,
	eSYS_STATE_PowerUp = eSYS_STATE_First,
	eSYS_STATE_Normal,
	eSYS_STATE_Error,
	eSYS_STATE_Last
}SYS_SystemStateTypeDef;


SYS_SystemStateTypeDef SYS_GetSystemState(void);
void SYS_PostEvent(SYS_EventTypeDef event);

#endif /* SYS_H_ */
