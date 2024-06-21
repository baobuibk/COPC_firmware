/*
 * scheduler.h
 *
 *  Created on: May 23, 2024
 *      Author: CAO HIEU
 */

#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "../Core/Common/basetypedef.h"

/* Exported types ------------------------------------------------------------*/
typedef void (*SCH_TASK) (void);
typedef void (*SCH_TimerCallback) (void);

#define SCH_TASK_HANDLE                 uint8_t
#define SCH_INVALID_TASK_HANDLE         0xFF
#define SCH_TIMER_HANDLE                uint8_t
#define SCH_INVALID_TIMER_HANDLE        0xFF

typedef enum SCH_TASK_TYPE
{
  SCH_TASK_NONE,
  SCH_TASK_SYNC,
  SCH_TASK_ASYNC
} SCH_TASK_TYPE;

typedef enum SCH_TIMER_TYPE
{
  SCH_TIMER_NONE = 0,
  SCH_TIMER_MONO,
  SCH_TIMER_PERIODIC
} SCH_TIMER_TYPE;

typedef enum SCH_TASK_PRIORITY
{
  SCH_TASK_PRIO_0 = 0,
  SCH_TASK_PRIO_1,
  SCH_TASK_PRIO_2,
  SCH_TASK_PRIO_3,
  SCH_TASK_PRIO_4,
  SCH_TASK_PRIO_5,
  SCH_TASK_PRIO_6,
  SCH_TASK_PRIO_7
} SCH_TASK_PRIORITY;

typedef struct SCH_TaskPropertyTypedef
{
  SCH_TASK_TYPE                 taskType;
  SCH_TASK_PRIORITY             taskPriority;
  uint32_t                      taskPeriodInMS;
  SCH_TASK                      taskFunction;
} SCH_TaskPropertyTypedef;

typedef struct SCH_TimerPropertyTypedef
{
  SCH_TIMER_TYPE                timerType;
  uint32_t                      timerPeriodInMS;
  SCH_TimerCallback             timerCallbackFunction;
} SCH_TimerPropertyTypedef;

typedef enum SCH_TaskStateTypedef
{
  TASK_StateHold = 0,
  TASK_StateReady
} SCH_TaskStateTypedef;

typedef enum SCH_TimerStateTypedef
{
  TIM_StateStop = 0,
  TIM_StateRun
} SCH_TimerStateTypedef;

typedef enum SCH_SoftTimerTypedef
{
  SCH_TIM_FIRST = 0,
  SCH_TIM_LED = SCH_TIM_FIRST,
  SCH_TIM_WDT,
  SCH_TIM_DELAY,
  SCH_TIM_ACK,
  SCH_TIM_RS422,
  SCH_TIM_PMU,
  SCH_TIM_PDU,
  SCH_TIM_IOU,
  SCH_TIM_LAST
} SCH_SoftTimerTypedef;

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void SCH_Initialize(void);

void SCH_TIM_Start(const SCH_SoftTimerTypedef timer, const uint32_t timeInMs);
uint16_t SCH_TIM_HasCompleted(const SCH_SoftTimerTypedef timer);

t_Status SCH_TASK_ResumeTask(SCH_TASK_HANDLE taskIndex);
t_Status SCH_TASK_StopTask(SCH_TASK_HANDLE taskIndex);
t_Status SCH_TASK_CreateTask(SCH_TASK_HANDLE* pHandle, SCH_TaskPropertyTypedef* pTaskProperty);
void SCH_RunSystemTickTimer(void);
void SCH_StartSchedular(void);
void SCH_StopSchedular(void);
void SCH_HandleScheduledTask(void);
void SCH_Delay(uint32_t delayInMs);
uint32_t SCH_SystemTick(void);

t_Status SCH_TIM_CreateTimer(SCH_TIMER_HANDLE* pHandle, SCH_TimerPropertyTypedef* pTimerProperty);
t_Status SCH_TIM_StopTimer(SCH_TIMER_HANDLE timerIndex);
t_Status SCH_TIM_RestartTimer(SCH_TIMER_HANDLE timerIndex);


#ifdef __cplusplus
}
#endif


#endif /* SCHEDULER_H_ */
