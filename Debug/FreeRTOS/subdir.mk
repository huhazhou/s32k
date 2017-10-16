################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.local

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS_QUOTED += \
"../FreeRTOS/croutine.c" \
"../FreeRTOS/event_groups.c" \
"../FreeRTOS/list.c" \
"../FreeRTOS/queue.c" \
"../FreeRTOS/tasks.c" \
"../FreeRTOS/timers.c" 

C_SRCS += \
../FreeRTOS/croutine.c \
../FreeRTOS/event_groups.c \
../FreeRTOS/list.c \
../FreeRTOS/queue.c \
../FreeRTOS/tasks.c \
../FreeRTOS/timers.c 

C_DEPS_QUOTED += \
"./FreeRTOS/croutine.d" \
"./FreeRTOS/event_groups.d" \
"./FreeRTOS/list.d" \
"./FreeRTOS/queue.d" \
"./FreeRTOS/tasks.d" \
"./FreeRTOS/timers.d" 

OBJS_QUOTED += \
"./FreeRTOS/croutine.o" \
"./FreeRTOS/event_groups.o" \
"./FreeRTOS/list.o" \
"./FreeRTOS/queue.o" \
"./FreeRTOS/tasks.o" \
"./FreeRTOS/timers.o" 

C_DEPS += \
./FreeRTOS/croutine.d \
./FreeRTOS/event_groups.d \
./FreeRTOS/list.d \
./FreeRTOS/queue.d \
./FreeRTOS/tasks.d \
./FreeRTOS/timers.d 

OBJS_OS_FORMAT += \
./FreeRTOS/croutine.o \
./FreeRTOS/event_groups.o \
./FreeRTOS/list.o \
./FreeRTOS/queue.o \
./FreeRTOS/tasks.o \
./FreeRTOS/timers.o 

OBJS += \
./FreeRTOS/croutine.o \
./FreeRTOS/event_groups.o \
./FreeRTOS/list.o \
./FreeRTOS/queue.o \
./FreeRTOS/tasks.o \
./FreeRTOS/timers.o 


# Each subdirectory must supply rules for building sources it contributes
FreeRTOS/croutine.o: ../FreeRTOS/croutine.c
	@echo 'Building file: $<'
	@echo 'Executing target #1 $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	arm-none-eabi-gcc "@FreeRTOS/croutine.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "FreeRTOS/croutine.o" "$<"
	@echo 'Finished building: $<'
	@echo ' '

FreeRTOS/event_groups.o: ../FreeRTOS/event_groups.c
	@echo 'Building file: $<'
	@echo 'Executing target #2 $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	arm-none-eabi-gcc "@FreeRTOS/event_groups.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "FreeRTOS/event_groups.o" "$<"
	@echo 'Finished building: $<'
	@echo ' '

FreeRTOS/list.o: ../FreeRTOS/list.c
	@echo 'Building file: $<'
	@echo 'Executing target #3 $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	arm-none-eabi-gcc "@FreeRTOS/list.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "FreeRTOS/list.o" "$<"
	@echo 'Finished building: $<'
	@echo ' '

FreeRTOS/queue.o: ../FreeRTOS/queue.c
	@echo 'Building file: $<'
	@echo 'Executing target #4 $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	arm-none-eabi-gcc "@FreeRTOS/queue.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "FreeRTOS/queue.o" "$<"
	@echo 'Finished building: $<'
	@echo ' '

FreeRTOS/tasks.o: ../FreeRTOS/tasks.c
	@echo 'Building file: $<'
	@echo 'Executing target #5 $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	arm-none-eabi-gcc "@FreeRTOS/tasks.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "FreeRTOS/tasks.o" "$<"
	@echo 'Finished building: $<'
	@echo ' '

FreeRTOS/timers.o: ../FreeRTOS/timers.c
	@echo 'Building file: $<'
	@echo 'Executing target #6 $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	arm-none-eabi-gcc "@FreeRTOS/timers.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "FreeRTOS/timers.o" "$<"
	@echo 'Finished building: $<'
	@echo ' '


