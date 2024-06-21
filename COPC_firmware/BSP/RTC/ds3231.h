/*
 * ds3231.h
 *
 *  Created on: May 31, 2024
 *      Author: CAO HIEU
 */

#ifndef RTC_DS3231_H_
#define RTC_DS3231_H_

#include <stdint.h>

void DS3231_Write(uint8_t reg, uint8_t data);
uint8_t DS3231_Read(uint8_t reg);
void DS3231_SetDateTime(uint8_t day, uint8_t date, uint8_t month, uint8_t year, uint8_t hour, uint8_t min, uint8_t sec);
void DS3231_GetDateTime(uint8_t *day, uint8_t *date, uint8_t *month, uint8_t *year, uint8_t *hour, uint8_t *min, uint8_t *sec);
float DS3231_GetTemperature(void);

#endif /* RTC_DS3231_H_ */
