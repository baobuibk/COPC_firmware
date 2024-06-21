/*
 * scheduler.c
 *
 *  Created on: May 23, 2024
 *      Author: CAO HIEU
 */

/* Standard Includes ---------------------------------------------------------*/
#include "scheduler.h"
#include <string.h>
#include "../BSP/SysTick/systick.h"
#include "stm32f4xx.h"
//#include "stm32f4xx_ll_gpio.h"

/* Private typedef -----------------------------------------------------------*/
typedef struct SCH_TaskContextTypedef
{
  SCH_TaskStateTypedef                  taskState;
  uint16_t                              taskFlag;
  uint32_t                              taskTick;
  const SCH_TaskPropertyTypedef*        pTaskProperty;
} SCH_TaskContextTypedef;

typedef struct SCH_TimerContextTypedef
{
  SCH_TimerStateTypedef                 timerState;
  uint16_t                              timerFlag;
  uint32_t                              timerTick;
  SCH_TimerPropertyTypedef*             pTimerProperty;
} SCH_TimerContextTypedef;

/* Private define ------------------------------------------------------------*/

#define MAX_TASK                        12
#define MAX_TIMERS                      12

/* Private macro -------------------------------------------------------------*/
//START STOP SYS_TICK?
#define SCH_START                       systick_timer_start()
#define SCH_STOP                        systick_timer_stop()

static SCH_TaskContextTypedef   s_TaskContext[MAX_TASK];
static uint8_t                  s_NumOfTaskScheduled;
static SCH_TimerContextTypedef  s_TimerContext[MAX_TIMERS];
static uint8_t                  s_NumOfTimers;

volatile uint32_t               s_SystemTick;
volatile uint32_t               s_SoftTimers[SCH_TIM_LAST];

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/



/* Public functions ----------------------------------------------------------*/
/*******************************************************************************
  * @name   SCH_Initialize
  * @brief  Function initializes schedular
  * @param  None
  * @retval None
  *****************************************************************************/

void SCH_Initialize(void)
{
  s_SystemTick = RESET;
  s_NumOfTaskScheduled = RESET;
  s_NumOfTimers = RESET;

  // Initial Scheduler Context
  memset((uint8_t*)&s_TaskContext[0], RESET, (sizeof(SCH_TaskContextTypedef) * MAX_TASK));
  memset((uint8_t*)&s_TimerContext[0], RESET, (sizeof(SCH_TimerContextTypedef) * MAX_TIMERS));
  memset((uint8_t*)&s_SoftTimers[0], RESET, (sizeof(uint32_t) * SCH_TIM_LAST));

    // Initialize Scheduler context
  systick_timer_init();
}


/*******************************************************************************
  * @name   SCH_TIM_Start
  * @brief  Start Soft timer
  * @param  const SCH_SoftTimerTypedef timer - type of soft timer
  *         const uint32_t timeInMs - time in mSec
  * @retval None
  *****************************************************************************/
void SCH_TIM_Start(const SCH_SoftTimerTypedef timer, const uint32_t timeInMs)
{
  if(timer < SCH_TIM_LAST)
  {
	//s_SoftTimers[timer] = timeInMs + s_SystemTick;
    s_SoftTimers[timer] = timeInMs;
  }
}


/*******************************************************************************
  * @name   SCH_TIM_HasCompleted
  * @brief  Function checks the completion of soft timer
  * @param  const SCH_SoftTimerTypedef timer - type of soft timer
  * @retval TRUE / FALSE
  *****************************************************************************/

uint16_t SCH_TIM_HasCompleted(const SCH_SoftTimerTypedef timer)
{
  return (s_SoftTimers[timer] == 0 ? 1:0 ) ;
}


/*******************************************************************************
  * @name   SCH_TASK_ResumeTask
  * @brief  Function resums suepended task
  * @param  SCH_TASK_HANDLE taskIndex - task handle
  * @retval status
  *****************************************************************************/
t_Status SCH_TASK_ResumeTask(SCH_TASK_HANDLE taskIndex)
{
  t_Status                      status = STS_ERROR;

  if(taskIndex < s_NumOfTaskScheduled)
  {
    // Get Task Context
    SCH_TaskContextTypedef* pTaskContext = &s_TaskContext[taskIndex];
    pTaskContext->taskState = TASK_StateReady;
    status = STS_DONE;
  }

  return status;

}

/*******************************************************************************
  * @name   SCH_TASK_StopTask
  * @brief  Function stops running task
  * @param  SCH_TASK_HANDLE taskIndex - task handle
  * @retval status
  *****************************************************************************/
t_Status SCH_TASK_StopTask(SCH_TASK_HANDLE taskIndex)
{
  t_Status                      status = STS_ERROR;

  if(taskIndex < s_NumOfTaskScheduled)
  {
    // Get Task Context
    SCH_TaskContextTypedef* pTaskContext = &s_TaskContext[taskIndex];
    pTaskContext->taskState = TASK_StateHold;
    status = STS_DONE;
  }

  return status;
}

/*******************************************************************************
  * @name   SCH_TASK_CreateTask
  * @brief  Function creates task
  * @param  SCH_TASK_HANDLE* pHandle - pointer to task handle
  *         SCH_TaskPropertyTypedef* pTaskProperty - pointer to task property
  * @retval status
  *****************************************************************************/
t_Status SCH_TASK_CreateTask(SCH_TASK_HANDLE* pHandle, SCH_TaskPropertyTypedef* pTaskProperty)
{
  t_Status                      status = STS_ERROR;

  // make sure that we have valid parameters
  if((pHandle) && (pTaskProperty))
  {
    // Check for number of task defined
    if(s_NumOfTaskScheduled < (MAX_TASK - 1))
    {
      SCH_TaskContextTypedef* pTaskContext = &s_TaskContext[s_NumOfTaskScheduled];
      // get task context
      // memcpy((uint8_t*)pTaskContext->pTaskProperty, (uint8_t*)pTaskProperty, sizeof(SCH_TaskPropertyTypedef));
      pTaskContext->pTaskProperty = pTaskProperty;
      // Make sure we are initializing other members of task context
      pTaskContext->taskFlag = FALSE;
      pTaskContext->taskTick = RESET;
      // Put task in Ready State
      pTaskContext->taskState = TASK_StateReady;

      // Give Task Handle back to caller
      *pHandle = s_NumOfTaskScheduled;

      s_NumOfTaskScheduled++;
      // We were able to register task with schedular
      status = STS_DONE;
    }
  }
  return status;
}

/*******************************************************************************
  * @name   SCH_TIM_CreateTimer
  * @brief  Function creates event timer
  * @param  SCH_TIMER_HANDLE* pHandle - timer handle
  *         SCH_TimerPropertyTypedef* pTimerProperty - pointer to timer property
  * @retval status
  *****************************************************************************/
t_Status SCH_TIM_CreateTimer(SCH_TIMER_HANDLE* pHandle, SCH_TimerPropertyTypedef* pTimerProperty)
{
  t_Status                      status = STS_ERROR;

  // make sure that we have valid parameters
  if((pHandle) && (pTimerProperty))
  {
    // Check for number of task defined
    if(s_NumOfTimers < (MAX_TIMERS - 1))
    {
      SCH_TimerContextTypedef* pTimerContext = &s_TimerContext[s_NumOfTimers];
      // get task context
      pTimerContext->pTimerProperty = pTimerProperty;
      // Make sure we are initializing other members of task context
      pTimerContext->timerState = TIM_StateStop;
      pTimerContext->timerFlag = FALSE;
      pTimerContext->timerTick = RESET;

      // Give Task Handle back to caller
      *pHandle = s_NumOfTimers;

      s_NumOfTimers++;
      // We were able to register timer with schedular
      status = STS_DONE;
    }
  }
  return status;
}

/*******************************************************************************
  * @name   SCH_TIM_RestartTimer
  * @brief  Function starts / restarts event timer
  * @param  SCH_TIMER_HANDLE timerIndex - event timer index
  * @retval status
  *****************************************************************************/
t_Status SCH_TIM_RestartTimer(SCH_TIMER_HANDLE timerIndex)
{
  t_Status                      status = STS_ERROR;

  if(timerIndex < s_NumOfTimers)
  {
    // Get Timer Context
    SCH_TimerContextTypedef* pTimerContext = &s_TimerContext[timerIndex];
    pTimerContext->timerTick = RESET;
    pTimerContext->timerState = TIM_StateRun;
    status = STS_DONE;
  }

  return status;
}

/*******************************************************************************
  * @name   SCH_TIM_StopTimer
  * @brief  Function stops event timer
  * @param  SCH_TIMER_HANDLE timerIndex - event timer handle
  * @retval status
  *****************************************************************************/
t_Status SCH_TIM_StopTimer(SCH_TIMER_HANDLE timerIndex)
{
  t_Status                      status = STS_ERROR;

  if(timerIndex < s_NumOfTimers)
  {
    // Get Timer Context
    SCH_TimerContextTypedef* pTimerContext = &s_TimerContext[timerIndex];
    pTimerContext->timerState = TIM_StateStop;
    status = STS_DONE;
  }

  return status;
}

/*******************************************************************************
  * @name   SCH_RunSystemTickTimer
  * @brief  Function handles system tick timer
  * @param  None
  * @retval None
  *****************************************************************************/
void SCH_RunSystemTickTimer(void)
{
  uint8_t                       taskIndex;
  SCH_TaskContextTypedef*       pTaskContext;
  uint8_t                       timerIndex;
  SCH_TimerContextTypedef*      pTimerContext;

  // Increment System Tick counter

  s_SystemTick++;
//  if(s_SystemTick > 100000){
//	  LL_GPIO_SetOutputPin(GPIOD, LL_GPIO_PIN_15);
//  }
  // Check Status of other Periodic Task
  for(taskIndex = 0; taskIndex < s_NumOfTaskScheduled; taskIndex++)
  {
    // Get Task Context
    pTaskContext = &s_TaskContext[taskIndex];

    // Check type and State of the task
    if((SCH_TASK_SYNC == pTaskContext->pTaskProperty->taskType) && (TASK_StateReady == pTaskContext->taskState))
    {
      // Increment task tick
      pTaskContext->taskTick += 1;

      // Check if we reached task period
      if(pTaskContext->taskTick >= pTaskContext->pTaskProperty->taskPeriodInMS)
      {
        // Yes
        // Reset Task tick timer
        pTaskContext->taskTick = RESET;
        // Enable Flag
        pTaskContext->taskFlag = TRUE;
      }
    }
  }

  // Check Status of other Periodic Task
  for(timerIndex = 0; timerIndex < s_NumOfTimers; timerIndex++)
  {
    // Get Task Context
    pTimerContext = &s_TimerContext[timerIndex];

    // Check type and State of the task
    if(TIM_StateRun == pTimerContext->timerState)
    {
      // Increment task tick
      pTimerContext->timerTick += 1;

      // Check if we reached task period
      if(pTimerContext->timerTick >= pTimerContext->pTimerProperty->timerPeriodInMS)
      {
        // Yes
        // Enable Flag
        pTimerContext->timerFlag = TRUE;
        // Reset tick timer
        pTimerContext->timerTick = RESET;
        // Check timer type and change the state
        pTimerContext->timerState = (SCH_TIMER_PERIODIC == pTimerContext->pTimerProperty->timerType)?TIM_StateRun:TIM_StateStop;
      }
    }
  }
  // Update software timer
   for(timerIndex = 0; timerIndex < SCH_TIM_LAST; timerIndex++)
 	if (s_SoftTimers[timerIndex] > 0)	s_SoftTimers[timerIndex] --;
}


/*******************************************************************************
  * @name   SCH_StartSchedular
  * @brief  Start schedular
  * @param  None
  * @retval None
  *****************************************************************************/
void SCH_StartSchedular(void)
{
  // Start Schedular..i.e. start system tick timer
  SCH_START;
}


/*******************************************************************************
  * @name   SCH_StopSchedular
  * @brief  Stop schedular
  * @param  None
  * @retval None
  *****************************************************************************/
void SCH_StopSchedular(void)
{
  // Stop Schedular..i.e. stop system tick timer
   SCH_STOP;
  // Initialize Schedular Context
  memset((uint8_t*)&s_TaskContext[0], RESET, (sizeof(SCH_TaskContextTypedef) * MAX_TASK));
  memset((uint8_t*)&s_TimerContext[0], RESET, (sizeof(SCH_TimerContextTypedef) * MAX_TIMERS));
  memset((uint8_t*)&s_SoftTimers[0], RESET, (sizeof(uint32_t) * SCH_TIM_LAST));
}

/*******************************************************************************
  * @name   SCH_HandleScheduledTask
  * @brief  Function handles scheduled task and timer events
  * @param  None
  * @retval None
  *****************************************************************************/
void SCH_HandleScheduledTask(void)
{
  uint8_t                       taskIndex;
  SCH_TaskContextTypedef*       pTaskContext;
  uint8_t                       timerIndex;
  SCH_TimerContextTypedef*      pTimerContext;

  // check for schedule flag
  for(taskIndex = 0; taskIndex < s_NumOfTaskScheduled; taskIndex++)
  {
    // Get Task Context
    pTaskContext = &s_TaskContext[taskIndex];

    // Check type and State of the task
    if((TRUE == pTaskContext->taskFlag) && (TASK_StateReady == pTaskContext->taskState))
    {
      pTaskContext->taskFlag = FALSE;
      if(pTaskContext->pTaskProperty->taskFunction)
      {
        pTaskContext->pTaskProperty->taskFunction();
      }
    }
  }

  // check for timer flag
  for(timerIndex = 0; timerIndex < s_NumOfTimers; timerIndex++)
  {
    // Get Timer Context
    pTimerContext = &s_TimerContext[timerIndex];

    // Check timer flag
    if(TRUE == pTimerContext->timerFlag)
    {
      pTimerContext->timerFlag = FALSE;
      if(pTimerContext->pTimerProperty->timerCallbackFunction)
      {
        pTimerContext->pTimerProperty->timerCallbackFunction();
      }
    }
  }
}


/*******************************************************************************
  * @name   SCH_Delay
  * @brief  Function to create a delay without affecting other tasks
  * @param  uint32_t delayInMs - delay time in milliseconds
  * @retval None
  *****************************************************************************/
void SCH_Delay(uint32_t delayInMs)
{
    // Start the delay timer
    SCH_TIM_Start(SCH_TIM_DELAY, delayInMs);

    // Wait until the delay timer completes
    while(!SCH_TIM_HasCompleted(SCH_TIM_DELAY));
}

/*******************************************************************************
  * @name   SCH_SystemTick
  * @brief  Function returns system ticks
  * @param  None
  * @retval system ticks
  *****************************************************************************/
uint32_t SCH_SystemTick(void)
{
  return s_SystemTick;
}

/*******************************************************************************
  * @name   SCH_SystemTick
  * @brief  Function returns system ticks
  * @param  None
  * @retval system ticks
  *****************************************************************************/


