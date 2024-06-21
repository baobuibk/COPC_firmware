/*
 * status_led.h
 *
 *  Created on: May 23, 2024
 *      Author: CAO HIEU
 */

#ifndef LED_STATUS_LED_H_
#define LED_STATUS_LED_H_

typedef	enum Led_DisplayStateTypedef
{POWERUP=0, NORMAL, POWERING_SUB, OVERCURRENT}Led_DisplayStateTypedef;
void  status_led_init(void);
void  status_led_set_status(Led_DisplayStateTypedef status);
void  status_led_create_task(void);
void command_send_splash(void);
#endif /* LED_STATUS_LED_H_ */
