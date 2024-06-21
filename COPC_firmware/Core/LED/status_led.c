/*
 * status_led.c
 *
 *  Created on: May 23, 2024
 *      Author: CAO HIEU
 */

#include "status_led.h"
#include "../../Scheduler/scheduler.h"
#include "stm32f4xx_ll_gpio.h"
#include "../../BSP/UART/uart.h"
#include "../../Core/CMDLine/command.h"
#include "main.h"
#include "../../BSP/RTC/ds3231.h"
#include <stdio.h>
uint8_t hour, min, sec;
char buffer[20];


/* Private define ------------------------------------------------------------*/
#define	POWERUP_PERIOD	1000
#define	POWER_NORMAL_OFF_PERIOD	3000
#define	POWER_NORMAL_ON_PERIOD	500
void	status_led_update(void);
static	void	status_led_normal(void);

/* Private typedef -----------------------------------------------------------*/
typedef struct Led_TaskContextTypedef
{
	SCH_TASK_HANDLE               taskHandle;
	SCH_TaskPropertyTypedef       taskProperty;
} Led_TaskContextTypedef;

typedef	struct StatusLed_CurrentStateTypedef
{
	uint8_t				led:1;
	Led_DisplayStateTypedef	state:7;
}StatusLed_CurrentStateTypedef;

/* Private variables ---------------------------------------------------------*/

static	StatusLed_CurrentStateTypedef	s_led_display_status = {0, POWERUP};
static Led_TaskContextTypedef           s_task_context =
{
	SCH_INVALID_TASK_HANDLE,                 // Will be updated by Schedular
	{
		SCH_TASK_SYNC,                      // taskType;
		SCH_TASK_PRIO_0,                    // taskPriority;
		500,                                // taskPeriodInMS;
		status_led_update                // taskFunction;
	}
};

static void status_led_on(void);
static void status_led_off(void);
static void status_led_powerup(void);

void	status_led_set_status(Led_DisplayStateTypedef status)
{
	s_led_display_status.state = status;
}

void status_led_init(void)
{
    // Initialize LED status variable
    s_led_display_status.led = 0;
    s_led_display_status.state = POWERUP;

    // Turn off LED initially
    status_led_off();
}

static void status_led_off(void)
{

	LL_GPIO_ResetOutputPin(LED_busy_GPIO_Port, LED_busy_Pin);
}

static void status_led_on(void)
{

	LL_GPIO_SetOutputPin(LED_busy_GPIO_Port, LED_busy_Pin);
}

void	status_led_update(void)
{
	switch (s_led_display_status.state) {
	case POWERUP:
		status_led_powerup();
		break;
	case NORMAL:
		status_led_normal();
		break;
	case POWERING_SUB:
		break;
	case OVERCURRENT:
		break;
	}
}

static void status_led_powerup(void)
{
    if (s_led_display_status.led == 1) // LED is ON
    {
        if (SCH_TIM_HasCompleted(SCH_TIM_LED))
        {
            s_led_display_status.led = 0;
//            command_send_splash();
//
//            DS3231_GetTime(&hour, &min, &sec);
//            sprintf(buffer, "Time: %02d:%02d:%02d\r\n", hour, min, sec);
//            Uart_sendstring(USART6, buffer);

//            uint8_t eject_value = LL_GPIO_IsInputPinSet(EJECT_GPIO_Port, EJECT_Pin);
//            char buffered[20];
//            sprintf(buffered, "EJECT value: %d\r\n", eject_value);
//            Uart_sendstring(USART6, buffered);

//            uint8_t pmu_int_value = LL_GPIO_IsInputPinSet(GPIOC, PMU_INT_MCU_Pin);
//            uint8_t pdu_int_value = LL_GPIO_IsInputPinSet(GPIOC, PDU_INT_MCU_Pin);
//            uint8_t iou_int_value = LL_GPIO_IsInputPinSet(GPIOC, IOU_INT_MCU_Pin);
//            uint8_t cam_int_value = LL_GPIO_IsInputPinSet(GPIOC, CAM_INT_MCU_Pin);
//            char buffered[100];
//            sprintf(buffered, "PMU_INT: %d, PDU_INT: %d, IOU_INT: %d, CAM_INT: %d\r\n",
//                    pmu_int_value, pdu_int_value, iou_int_value, cam_int_value);
//            Uart_sendstring(USART6, buffered);

            status_led_off();
//            command_send_splash();
            SCH_TIM_Start(SCH_TIM_LED, POWERUP_PERIOD); // restart

        }
    }
    else if (s_led_display_status.led == 0) // LED is OFF
    {
        if (SCH_TIM_HasCompleted(SCH_TIM_LED))
        {
            s_led_display_status.led = 1;
            status_led_on();
            SCH_TIM_Start(SCH_TIM_LED, POWERUP_PERIOD); // restart
        }
    }
}


//void command_send_splash(void)
//{
//	usart6_send_string("Hello World\r\n");
//	LL_GPIO_SetOutputPin(GPIOD, LL_GPIO_PIN_15);
//	usart6_send_string(".......... / ___| _ __   __ _  ___ ___| |    __ _| |__   ..........\r\n");
//
//	usart6_send_string(".......... \\___ \\| '_ \\ / _` |/ __/ _ \\ |   / _` | '_ \\  ..........\r\n");
//	usart6_send_string("..........  ___) | |_) | (_| | (_|  __/ |__| (_| | |_) | ..........\r\n");
//	usart6_send_string(".......... |____/| .__/ \\__,_|\\___\\___|_____\\__,_|_.__/  ..........\r\n");
//	usart6_send_string("..........       |_|                                     ..........\r\n");
//	usart6_send_string("> ");
//}
//


static void status_led_normal(void)
{
//	if (s_led_display_status.led == 1)
//	{
//		if (SCH_TIM_HasCompleted(SCH_TIM_LED))
//			{
//				s_led_display_status.led = 0;
//				status_led_off();
//				SCH_TIM_Start(SCH_TIM_LED,POWER_NORMAL_OFF_PERIOD);	//restart
//			}
//	}
//	else if (s_led_display_status.led == 1)
//	{
//		if (SCH_TIM_HasCompleted(SCH_TIM_LED))
//		{
//			s_led_display_status.led = 1;
//			status_led_on();
//			SCH_TIM_Start(SCH_TIM_LED,POWER_NORMAL_ON_PERIOD);	//restart
//		}
//	}
}

void	status_led_create_task(void)
{
	SCH_TASK_CreateTask(&s_task_context.taskHandle, &s_task_context.taskProperty);
}



