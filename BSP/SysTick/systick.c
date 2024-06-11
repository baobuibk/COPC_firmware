/*
 * systick.c
 *
 *  Created on: May 23, 2024
 *      Author: CAO HIEU
 */

#include "systick.h"
#include "stm32f4xx.h"

void systick_timer_start(void)
{
    // Reset SysTick counter value
   // SysTick->VAL = 0;

    // Enable SysTick counter
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
}

void systick_timer_stop(void)
{
    // Disable SysTick counter
   // SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
}

void systick_timer_init(void)
{
    // Set SysTick to trigger interrupt every 1ms
    SysTick_Config(SystemCoreClock/1000);
}
