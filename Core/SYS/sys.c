/*
 * sys.c
 *
 *  Created on: May 23, 2024
 *      Author: CAO HIEU
 */

#include "sys.h"
#include "stm32f4xx.h"

/* Private typedef -----------------------------------------------------------*/
typedef void (*t_tpAppEventHandler) (void);

static SYS_SystemStateTypeDef           s_SystemState = eSYS_STATE_PowerUp;
static FlagStatus                       s_AppEvents[eSYS_EVENT_MAX] =
{
  RESET,                // eSYS_EVENT_EJECT,
  RESET,                // eSYS_EVENT_INT_I2C?,
};

/* Private functions ---------------------------------------------------------*/
//static void SYS_handle_EJECT(void);
//static void SYS_handle_INT_I2C(void);

/* Public functions ----------------------------------------------------------*/
/*******************************************************************************
  * @name   SYS_GetSystemState
  * @brief  Function returns the current state of the system
  * @param  None
  * @retval SYS_SystemStateTypedef - system state
  *****************************************************************************/
SYS_SystemStateTypeDef SYS_GetSystemState(void)
{
  return(s_SystemState);
}

/*******************************************************************************
  * @name   SYS_PostEvent
  * @brief  Function Sets the requested Application Event
  * @param  SYS_EventTypedef event - event to be posted
  * @retval None
  *****************************************************************************/
void SYS_PostEvent(SYS_EventTypeDef event)
{
  // check for the requested event
  if(event < eSYS_EVENT_MAX)
  {
    s_AppEvents[event] = SET;
  }
}


/*******************************************************************************
  * @name   SYS_HandleEvents
  * @brief  Function handles application Events
  * @param  None
  * @retval None
  *****************************************************************************/

//static void	SYS_handle_events(void)
//{
//	const	t_tpAppEventHandler	ftpEventHandler[eSYS_EVENT_MAX] =
//	{
//		SYS_handle_EJECT,
//		SYS_handle_INT_I2C
//	};
//	SYS_EventTypeDef	eventIndex;
//	for(eventIndex = eSYS_EVENT_First;eventIndex < eSYS_EVENT_MAX;eventIndex++)
//	{
//		if (s_AppEvents[eventIndex] == SET)
//		{
//			ftpEventHandler[eventIndex]();
//		}
//	}
//}
