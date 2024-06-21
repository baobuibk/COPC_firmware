#include "stm32f4xx_ll_i2c.h"
#include "stm32f4xx_ll_rcc.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_bus.h"
#include "ds3231.h"

#define DS3231_ADDRESS 0xD0

void DS3231_Write(uint8_t reg, uint8_t data)
{
    while(LL_I2C_IsActiveFlag_BUSY(I2C1));
    LL_I2C_GenerateStartCondition(I2C1);
    while(!LL_I2C_IsActiveFlag_SB(I2C1));
    LL_I2C_TransmitData8(I2C1, DS3231_ADDRESS);
    while(!LL_I2C_IsActiveFlag_ADDR(I2C1));
    LL_I2C_ClearFlag_ADDR(I2C1);
    LL_I2C_TransmitData8(I2C1, reg);
    while(!LL_I2C_IsActiveFlag_TXE(I2C1));
    LL_I2C_TransmitData8(I2C1, data);
    while(!LL_I2C_IsActiveFlag_TXE(I2C1));
    LL_I2C_GenerateStopCondition(I2C1);
}

uint8_t DS3231_Read(uint8_t reg)
{
    uint8_t data = 0;
    while(LL_I2C_IsActiveFlag_BUSY(I2C1));
    LL_I2C_GenerateStartCondition(I2C1);
    while(!LL_I2C_IsActiveFlag_SB(I2C1));
    LL_I2C_TransmitData8(I2C1, DS3231_ADDRESS);
    while(!LL_I2C_IsActiveFlag_ADDR(I2C1));
    LL_I2C_ClearFlag_ADDR(I2C1);
    LL_I2C_TransmitData8(I2C1, reg);
    while(!LL_I2C_IsActiveFlag_TXE(I2C1));
    LL_I2C_GenerateStartCondition(I2C1);
    while(!LL_I2C_IsActiveFlag_SB(I2C1));
    LL_I2C_TransmitData8(I2C1, DS3231_ADDRESS | 1);
    while(!LL_I2C_IsActiveFlag_ADDR(I2C1));
    LL_I2C_ClearFlag_ADDR(I2C1);
    LL_I2C_AcknowledgeNextData(I2C1, LL_I2C_NACK);
    while(!LL_I2C_IsActiveFlag_RXNE(I2C1));
    data = LL_I2C_ReceiveData8(I2C1);
    LL_I2C_GenerateStopCondition(I2C1);
    return data;
}

void DS3231_SetDateTime(uint8_t day, uint8_t date, uint8_t month, uint8_t year, uint8_t hour, uint8_t min, uint8_t sec)
{
    DS3231_Write(0x00, ((sec / 10) << 4) | (sec % 10));
    DS3231_Write(0x01, ((min / 10) << 4) | (min % 10));
    DS3231_Write(0x02, ((hour / 10) << 4) | (hour % 10));
    DS3231_Write(0x03, day);
    DS3231_Write(0x04, ((date / 10) << 4) | (date % 10));
    DS3231_Write(0x05, ((month / 10) << 4) | (month % 10));
    DS3231_Write(0x06, ((year / 10) << 4) | (year % 10));
}

void DS3231_GetDateTime(uint8_t *day, uint8_t *date, uint8_t *month, uint8_t *year, uint8_t *hour, uint8_t *min, uint8_t *sec)
{
    *sec = DS3231_Read(0x00);
    *min = DS3231_Read(0x01);
    *hour = DS3231_Read(0x02);
    *day = DS3231_Read(0x03);
    *date = DS3231_Read(0x04);
    *month = DS3231_Read(0x05);
    *year = DS3231_Read(0x06);

    *sec = ((*sec >> 4) * 10) + (*sec & 0x0F);
    *min = ((*min >> 4) * 10) + (*min & 0x0F);
    *hour = ((*hour >> 4) * 10) + (*hour & 0x0F);
    *date = ((*date >> 4) * 10) + (*date & 0x0F);
    *month = ((*month >> 4) * 10) + (*month & 0x0F);
    *year = ((*year >> 4) * 10) + (*year & 0x0F);
}

float DS3231_GetTemperature(void)
{
    uint8_t temp_msb = DS3231_Read(0x11);
    uint8_t temp_lsb = DS3231_Read(0x12);
    int16_t temp = (temp_msb << 8) | temp_lsb;
    float temperature = temp / 256.0;
    return temperature;
}
