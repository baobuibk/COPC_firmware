/*
 * watchdog.h
 *
 *  Created on: May 24, 2024
 *      Author: CAO HIEU
 */

#ifndef WATCHDOG_WATCHDOG_H_
#define WATCHDOG_WATCHDOG_H_

#include "../../Scheduler/scheduler.h"
#include "../Common//basetypedef.h"
#include "stm32f4xx_ll_gpio.h"
#include "main.h"


void WDT_init(void);
void WDT_create_task(void);
void status_wdt_update(void);

#endif /* WATCHDOG_WATCHDOG_H_ */
