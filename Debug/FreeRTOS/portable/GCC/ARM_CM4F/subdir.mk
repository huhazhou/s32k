################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../../../../makefile.local

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS_QUOTED += \
"../FreeRTOS/portable/GCC/ARM_CM4F/port.c" 

C_SRCS += \
../FreeRTOS/portable/GCC/ARM_CM4F/port.c 

C_DEPS_QUOTED += \
"./FreeRTOS/portable/GCC/ARM_CM4F/port.d" 

OBJS_QUOTED += \
"./FreeRTOS/portable/GCC/ARM_CM4F/port.o" 

C_DEPS += \
./FreeRTOS/portable/GCC/ARM_CM4F/port.d 

OBJS_OS_FORMAT += \
./FreeRTOS/portable/GCC/ARM_CM4F/port.o 

OBJS += \
./FreeRTOS/portable/GCC/ARM_CM4F/port.o 


# Each subdirectory must supply rules for building sources it contributes
FreeRTOS/portable/GCC/ARM_CM4F/port.o: ../FreeRTOS/portable/GCC/ARM_CM4F/port.c
	@echo 'Building file: $<'
	@echo 'Executing target #7 $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	arm-none-eabi-gcc "@FreeRTOS/portable/GCC/ARM_CM4F/port.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "FreeRTOS/portable/GCC/ARM_CM4F/port.o" "$<"
	@echo 'Finished building: $<'
	@echo ' '


