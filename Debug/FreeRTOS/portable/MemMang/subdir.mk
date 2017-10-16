################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../../../makefile.local

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS_QUOTED += \
"../FreeRTOS/portable/MemMang/heap_2.c" 

C_SRCS += \
../FreeRTOS/portable/MemMang/heap_2.c 

C_DEPS_QUOTED += \
"./FreeRTOS/portable/MemMang/heap_2.d" 

OBJS_QUOTED += \
"./FreeRTOS/portable/MemMang/heap_2.o" 

C_DEPS += \
./FreeRTOS/portable/MemMang/heap_2.d 

OBJS_OS_FORMAT += \
./FreeRTOS/portable/MemMang/heap_2.o 

OBJS += \
./FreeRTOS/portable/MemMang/heap_2.o 


# Each subdirectory must supply rules for building sources it contributes
FreeRTOS/portable/MemMang/heap_2.o: ../FreeRTOS/portable/MemMang/heap_2.c
	@echo 'Building file: $<'
	@echo 'Executing target #8 $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	arm-none-eabi-gcc "@FreeRTOS/portable/MemMang/heap_2.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "FreeRTOS/portable/MemMang/heap_2.o" "$<"
	@echo 'Finished building: $<'
	@echo ' '


