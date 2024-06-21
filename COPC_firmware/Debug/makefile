################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include ThirdParty/libfsp/subdir.mk
-include Scheduler/subdir.mk
-include Drivers/STM32F4xx_HAL_Driver/Src/subdir.mk
-include Core/WATCHDOG/subdir.mk
-include Core/Startup/subdir.mk
-include Core/Src/subdir.mk
-include Core/SYS/subdir.mk
-include Core/LED/subdir.mk
-include Core/GPS/subdir.mk
-include Core/CPOC/subdir.mk
-include Core/CMDLine/cmd_PMU/subdir.mk
-include Core/CMDLine/cmd_PDU/subdir.mk
-include Core/CMDLine/cmd_IOU/subdir.mk
-include Core/CMDLine/cmd_CAM/subdir.mk
-include Core/CMDLine/ACK_packet/subdir.mk
-include Core/CMDLine/subdir.mk
-include BSP/uartstdio/subdir.mk
-include BSP/UART/subdir.mk
-include BSP/SysTick/subdir.mk
-include BSP/RTC/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := cpoc_firmware
BUILD_ARTIFACT_EXTENSION := elf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
cpoc_firmware.elf \

MAP_FILES += \
cpoc_firmware.map \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
cpoc_firmware.list \

OBJCOPY_HEX += \
cpoc_firmware.hex \

OBJCOPY_BIN += \
cpoc_firmware.bin \


# All Target
all: main-build

# Main-build Target
main-build: cpoc_firmware.elf secondary-outputs

# Tool invocations
cpoc_firmware.elf cpoc_firmware.map: $(OBJS) $(USER_OBJS) D:\HTN\COPC_firmware\STM32F405RGTX_FLASH.ld makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-gcc -o "cpoc_firmware.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m4 -T"D:\HTN\COPC_firmware\STM32F405RGTX_FLASH.ld" --specs=nosys.specs -Wl,-Map="cpoc_firmware.map" -Wl,--gc-sections -static --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -u _printf_float -u _scanf_float -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

cpoc_firmware.list: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "cpoc_firmware.list"
	@echo 'Finished building: $@'
	@echo ' '

cpoc_firmware.hex: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objcopy  -O ihex $(EXECUTABLES) "cpoc_firmware.hex"
	@echo 'Finished building: $@'
	@echo ' '

cpoc_firmware.bin: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objcopy  -O binary $(EXECUTABLES) "cpoc_firmware.bin"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) cpoc_firmware.bin cpoc_firmware.elf cpoc_firmware.hex cpoc_firmware.list cpoc_firmware.map default.size.stdout
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST) $(OBJCOPY_HEX) $(OBJCOPY_BIN)

fail-specified-linker-script-missing:
	@echo 'Error: Cannot find the specified linker script. Check the linker settings in the build configuration.'
	@exit 2

warn-no-linker-script-specified:
	@echo 'Warning: No linker script specified. Check the linker settings in the build configuration.'

.PHONY: all clean dependents main-build fail-specified-linker-script-missing warn-no-linker-script-specified

-include ../makefile.targets
