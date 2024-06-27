/*
 * img_i2c.h
 *
 *  Created on: Jun 26, 2024
 *      Author: CAO HIEU
 */

#ifndef IMG_I2C_IMG_I2C_H_
#define IMG_I2C_IMG_I2C_H_

#define IMG_ADDRESS 0x13
#define IMG_SIZE 7296
#define BLOCK_SIZE 128
#define NUM_BLOCKS (IMG_SIZE / BLOCK_SIZE)


void I2C3_IRQHandler(void);
void I2C_img_create_task(void);

#endif /* IMG_I2C_IMG_I2C_H_ */
