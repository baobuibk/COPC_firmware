################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/CMDLine/ACK_packet/ACKsend_packet.c 

OBJS += \
./Core/CMDLine/ACK_packet/ACKsend_packet.o 

C_DEPS += \
./Core/CMDLine/ACK_packet/ACKsend_packet.d 


# Each subdirectory must supply rules for building sources it contributes
Core/CMDLine/ACK_packet/%.o Core/CMDLine/ACK_packet/%.su Core/CMDLine/ACK_packet/%.cyclo: ../Core/CMDLine/ACK_packet/%.c Core/CMDLine/ACK_packet/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DSTM32F405xx -DUSE_FULL_LL_DRIVER -DHSE_VALUE=8000000 -DHSE_STARTUP_TIMEOUT=100 -DLSE_STARTUP_TIMEOUT=5000 -DLSE_VALUE=32768 -DEXTERNAL_CLOCK_VALUE=12288000 -DHSI_VALUE=16000000 -DLSI_VALUE=32000 -DVDD_VALUE=3300 -DPREFETCH_ENABLE=1 -DINSTRUCTION_CACHE_ENABLE=1 -DDATA_CACHE_ENABLE=1 -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-CMDLine-2f-ACK_packet

clean-Core-2f-CMDLine-2f-ACK_packet:
	-$(RM) ./Core/CMDLine/ACK_packet/ACKsend_packet.cyclo ./Core/CMDLine/ACK_packet/ACKsend_packet.d ./Core/CMDLine/ACK_packet/ACKsend_packet.o ./Core/CMDLine/ACK_packet/ACKsend_packet.su

.PHONY: clean-Core-2f-CMDLine-2f-ACK_packet

