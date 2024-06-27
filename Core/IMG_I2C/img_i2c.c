/*
 * img_i2c.c
 *
 *  Created on: Jun 26, 2024
 *      Author: CAO HIEU
 */
#include "stm32f4xx_ll_i2c.h"
#include "stm32f4xx_ll_rcc.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_bus.h"

#include "img_i2c.h"
#include "../../Scheduler/scheduler.h"
#include <stm32f4xx_ll_i2c.h>
//#include "../../Core/CMDLine/global_vars.h"
#include <stdio.h>
#include "../../BSP/UART/uart.h"

static uint8_t imgData[IMG_SIZE];
static uint16_t imgReadIndex = 0;
static uint16_t currentBlock = 0;

/*Private function prototypes*/
static void I2C_img_task_update(void);

#define IMG_PERIOD 1000

/*Private typedef*/
typedef struct I2C_img_TaskContextTypedef
{
    SCH_TASK_HANDLE taskHandle;
    SCH_TaskPropertyTypedef taskProperty;
} I2C_img_TaskContextTypedef;

static I2C_img_TaskContextTypedef i2c_img_task_context =
{
    SCH_INVALID_TASK_HANDLE, // Will be updated by Scheduler
    {
        SCH_TASK_SYNC, // taskType;
        SCH_TASK_PRIO_0, // taskPriority;
        10, // taskPeriodInMS;
        I2C_img_task_update // taskFunction;
    }
};

uint8_t img_read_enable = 0;
static void I2C_img_task_update(void)
{
  //  if (img_read_enable)
    if (img_read_enable)
    {
        if (currentBlock < NUM_BLOCKS)
        {
            if (imgReadIndex == 0)
            {
                // Start reading the current block
    //            LL_I2C_GenerateStartCondition(I2C3);
  //              LL_I2C_TransmitData8(I2C3, IMG_ADDRESS | 0x01);
     //           while (!LL_I2C_IsActiveFlag_ADDR(I2C3));
//                LL_I2C_ClearFlag_ADDR(I2C3);
//                LL_I2C_EnableIT_RX(I2C3);


                LL_I2C_GenerateStartCondition(I2C3);

                LL_I2C_TransmitData8(I2C3, IMG_ADDRESS);
                LL_I2C_ClearFlag_ADDR(I2C3);
                LL_I2C_GenerateStopCondition(I2C3);
            }
        }
        else
        {
            // All blocks have been read
            LL_I2C_DisableIT_RX(I2C3);
            LL_I2C_GenerateStopCondition(I2C3);
            img_read_enable = 0;
            // Process the received data
            // Switch block ở đây
            // ...
            for (int i = 0; i < IMG_SIZE; i++) {
                Uart_write(UART4, imgData[i]);
            }
        }

        // Check if 1 second has elapsed
        if (SCH_TIM_HasCompleted(SCH_TIM_IMG))
        {
            // Reset the read process
            imgReadIndex = 0;
            currentBlock = 0;
            SCH_TIM_Start(SCH_TIM_IMG, IMG_PERIOD);
        }
    }
    else
    {
        // Reset the read process
        imgReadIndex = 0;
        currentBlock = 0;
        SCH_TIM_Start(SCH_TIM_IMG, IMG_PERIOD);
    }
}


void I2C_img_create_task(void)
{
    SCH_TASK_CreateTask(&i2c_img_task_context.taskHandle, &i2c_img_task_context.taskProperty);
}

void I2C3_IRQHandler(void)
{
    if (LL_I2C_IsActiveFlag_RXNE(I2C3))
    {
        // Read received data
        imgData[currentBlock * BLOCK_SIZE + imgReadIndex] = LL_I2C_ReceiveData8(I2C3);
        imgReadIndex++;

        if (imgReadIndex >= BLOCK_SIZE)
        {
            // Current block is fully read
            LL_I2C_DisableIT_RX(I2C3);
            LL_I2C_GenerateStopCondition(I2C3);
            imgReadIndex = 0;
            currentBlock++;
        }
    }
}
