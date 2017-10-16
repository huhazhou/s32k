################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../../makefile.local

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS_QUOTED += \
"../freertos/portable/port.c" 

C_SRCS += \
../freertos/portable/port.c 

C_DEPS_QUOTED += \
"./freertos/portable/port.d" 

OBJS_QUOTED += \
"./freertos/portable/port.o" 

C_DEPS += \
./freertos/portable/port.d 

OBJS_OS_FORMAT += \
./freertos/portable/port.o 

OBJS += \
./freertos/portable/port.o 


# Each subdirectory must supply rules for building sources it contributes
freertos/portable/port.o: ../freertos/portable/port.c
	@echo 'Building file: $<'
	@echo 'Executing target #18 $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	arm-none-eabi-gcc "@freertos/portable/port.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "freertos/portable/port.o" "$<"
	@echo 'Finished building: $<'
	@echo ' '


