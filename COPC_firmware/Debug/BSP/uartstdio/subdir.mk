################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../BSP/uartstdio/uartstdio.c 

OBJS += \
./BSP/uartstdio/uartstdio.o 

C_DEPS += \
./BSP/uartstdio/uartstdio.d 


# Each subdirectory must supply rules for building sources it contributes
BSP/uartstdio/%.o BSP/uartstdio/%.su BSP/uartstdio/%.cyclo: ../BSP/uartstdio/%.c BSP/uartstdio/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DSTM32F405xx -DUSE_FULL_LL_DRIVER -DHSE_VALUE=8000000 -DHSE_STARTUP_TIMEOUT=100 -DLSE_STARTUP_TIMEOUT=5000 -DLSE_VALUE=32768 -DEXTERNAL_CLOCK_VALUE=12288000 -DHSI_VALUE=16000000 -DLSI_VALUE=32000 -DVDD_VALUE=3300 -DPREFETCH_ENABLE=1 -DINSTRUCTION_CACHE_ENABLE=1 -DDATA_CACHE_ENABLE=1 -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-BSP-2f-uartstdio

clean-BSP-2f-uartstdio:
	-$(RM) ./BSP/uartstdio/uartstdio.cyclo ./BSP/uartstdio/uartstdio.d ./BSP/uartstdio/uartstdio.o ./BSP/uartstdio/uartstdio.su

.PHONY: clean-BSP-2f-uartstdio

