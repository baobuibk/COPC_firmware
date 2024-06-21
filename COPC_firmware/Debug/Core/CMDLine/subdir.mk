################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/CMDLine/cmdline.c \
../Core/CMDLine/command.c \
../Core/CMDLine/rs422.c 

OBJS += \
./Core/CMDLine/cmdline.o \
./Core/CMDLine/command.o \
./Core/CMDLine/rs422.o 

C_DEPS += \
./Core/CMDLine/cmdline.d \
./Core/CMDLine/command.d \
./Core/CMDLine/rs422.d 


# Each subdirectory must supply rules for building sources it contributes
Core/CMDLine/%.o Core/CMDLine/%.su Core/CMDLine/%.cyclo: ../Core/CMDLine/%.c Core/CMDLine/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DSTM32F405xx -DUSE_FULL_LL_DRIVER -DHSE_VALUE=8000000 -DHSE_STARTUP_TIMEOUT=100 -DLSE_STARTUP_TIMEOUT=5000 -DLSE_VALUE=32768 -DEXTERNAL_CLOCK_VALUE=12288000 -DHSI_VALUE=16000000 -DLSI_VALUE=32000 -DVDD_VALUE=3300 -DPREFETCH_ENABLE=1 -DINSTRUCTION_CACHE_ENABLE=1 -DDATA_CACHE_ENABLE=1 -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-CMDLine

clean-Core-2f-CMDLine:
	-$(RM) ./Core/CMDLine/cmdline.cyclo ./Core/CMDLine/cmdline.d ./Core/CMDLine/cmdline.o ./Core/CMDLine/cmdline.su ./Core/CMDLine/command.cyclo ./Core/CMDLine/command.d ./Core/CMDLine/command.o ./Core/CMDLine/command.su ./Core/CMDLine/rs422.cyclo ./Core/CMDLine/rs422.d ./Core/CMDLine/rs422.o ./Core/CMDLine/rs422.su

.PHONY: clean-Core-2f-CMDLine

