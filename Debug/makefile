################################################################################
# Automatically-generated file. Do not edit!
################################################################################

#Progress monitor hint: 25
first : all
-include ../makefile.init

# This file contains definitions of environment variables used in the makefiles and .args files if exist.
-include makefile.local

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include FreeRTOS/subdir.mk
-include FreeRTOS/portable/GCC/ARM_CM4F/subdir.mk
-include FreeRTOS/portable/MemMang/subdir.mk
-include Project_Settings/Linker_Files/subdir.mk
-include Project_Settings/Startup_Code/subdir.mk
-include bl/subdir.mk
-include fatfs/subdir.mk
-include src/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
dp1000.elf \

EXECUTABLES_QUOTED += \
"dp1000.elf" \

EXECUTABLES_OS_FORMAT += \
dp1000.elf \

SECONDARY_FLASH += \
dp1000.srec \

SECONDARY_FLASH_QUOTED += \
"dp1000.srec" \

SECONDARY_FLASH_OS_FORMAT += \
dp1000.srec \

SECONDARY_SIZE += \
dp1000.siz \

SECONDARY_SIZE_QUOTED += \
"dp1000.siz" \

SECONDARY_SIZE_OS_FORMAT += \
dp1000.siz \


# All Target
all: warning-messages explicit-dependencies dp1000.elf secondary-outputs

# Tool invocations
dp1000.elf: $(OBJS) /home/houxd/workspaceS32DS.ARM.V20/dp1000/Project_Settings/Linker_Files/S32K1xx_flash.ld $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Executing target #26 $@'
	@echo 'Invoking: Standard S32DS C++ Linker'
	arm-none-eabi-g++ -o "dp1000.elf" "@dp1000.args"  $(USER_OBJS_QUOTED) $(LIBS_QUOTED)
	@echo 'Finished building target: $@'
	@echo ' '

dp1000.srec: $(EXECUTABLES)
	@echo 'Executing target #27 $@'
	@echo 'Invoking: Standard S32DS Create Flash Image'
	arm-none-eabi-objcopy -O srec $(EXECUTABLES)  "dp1000.srec"
	@echo 'Finished building: $@'
	@echo ' '

dp1000.siz: $(EXECUTABLES)
	@echo 'Executing target #28 $@'
	@echo 'Invoking: Standard S32DS Print Size'
	arm-none-eabi-size --format=berkeley $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(SECONDARY_FLASH_QUOTED) $(CPP_DEPS_QUOTED) $(CC_DEPS_QUOTED) $(CXX_DEPS_QUOTED) $(C_UPPER_DEPS_QUOTED) $(SECONDARY_SIZE_QUOTED) $(C++_DEPS_QUOTED) ./*/*.d ./*/*/*.d ./*/*/*/*.d ./*/*/*/*/*.d  ./*/*.o ./*/*/*.o ./*/*/*/*.o ./*/*/*/*/*.o  $(EXECUTABLES_QUOTED)
	-@echo ' '

secondary-outputs: $(SECONDARY_FLASH) $(SECONDARY_SIZE)

.PHONY: all clean dependents explicit-dependencies warning-messages
.SECONDARY:

-include ../makefile.targets

explicit-dependencies: 

warning-messages: 

