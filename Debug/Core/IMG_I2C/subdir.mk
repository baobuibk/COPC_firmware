################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/IMG_I2C/img_i2c.c 

OBJS += \
./Core/IMG_I2C/img_i2c.o 

C_DEPS += \
./Core/IMG_I2C/img_i2c.d 


# Each subdirectory must supply rules for building sources it contributes
Core/IMG_I2C/%.o Core/IMG_I2C/%.su Core/IMG_I2C/%.cyclo: ../Core/IMG_I2C/%.c Core/IMG_I2C/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DSTM32F405xx -DUSE_FULL_LL_DRIVER -DHSE_VALUE=8000000 -DHSE_STARTUP_TIMEOUT=100 -DLSE_STARTUP_TIMEOUT=5000 -DLSE_VALUE=32768 -DEXTERNAL_CLOCK_VALUE=12288000 -DHSI_VALUE=16000000 -DLSI_VALUE=32000 -DVDD_VALUE=3300 -DPREFETCH_ENABLE=1 -DINSTRUCTION_CACHE_ENABLE=1 -DDATA_CACHE_ENABLE=1 -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-IMG_I2C

clean-Core-2f-IMG_I2C:
	-$(RM) ./Core/IMG_I2C/img_i2c.cyclo ./Core/IMG_I2C/img_i2c.d ./Core/IMG_I2C/img_i2c.o ./Core/IMG_I2C/img_i2c.su

.PHONY: clean-Core-2f-IMG_I2C

