################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.local

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS_QUOTED += \
"../freertos/croutine.c" \
"../freertos/event_groups.c" \
"../freertos/list.c" \
"../freertos/queue.c" \
"../freertos/tasks.c" \
"../freertos/timers.c" 

C_SRCS += \
../freertos/croutine.c \
../freertos/event_groups.c \
../freertos/list.c \
../freertos/queue.c \
../freertos/tasks.c \
../freertos/timers.c 

C_DEPS_QUOTED += \
"./freertos/croutine.d" \
"./freertos/event_groups.d" \
"./freertos/list.d" \
"./freertos/queue.d" \
"./freertos/tasks.d" \
"./freertos/timers.d" 

OBJS_QUOTED += \
"./freertos/croutine.o" \
"./freertos/event_groups.o" \
"./freertos/list.o" \
"./freertos/queue.o" \
"./freertos/tasks.o" \
"./freertos/timers.o" 

C_DEPS += \
./freertos/croutine.d \
./freertos/event_groups.d \
./freertos/list.d \
./freertos/queue.d \
./freertos/tasks.d \
./freertos/timers.d 

OBJS_OS_FORMAT += \
./freertos/croutine.o \
./freertos/event_groups.o \
./freertos/list.o \
./freertos/queue.o \
./freertos/tasks.o \
./freertos/timers.o 

OBJS += \
./freertos/croutine.o \
./freertos/event_groups.o \
./freertos/list.o \
./freertos/queue.o \
./freertos/tasks.o \
./freertos/timers.o 


# Each subdirectory must supply rules for building sources it contributes
freertos/croutine.o: ../freertos/croutine.c
	@echo 'Building file: $<'
	@echo 'Executing target #12 $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	arm-none-eabi-gcc "@freertos/croutine.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "freertos/croutine.o" "$<"
	@echo 'Finished building: $<'
	@echo ' '

freertos/event_groups.o: ../freertos/event_groups.c
	@echo 'Building file: $<'
	@echo 'Executing target #13 $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	arm-none-eabi-gcc "@freertos/event_groups.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "freertos/event_groups.o" "$<"
	@echo 'Finished building: $<'
	@echo ' '

freertos/list.o: ../freertos/list.c
	@echo 'Building file: $<'
	@echo 'Executing target #14 $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	arm-none-eabi-gcc "@freertos/list.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "freertos/list.o" "$<"
	@echo 'Finished building: $<'
	@echo ' '

freertos/queue.o: ../freertos/queue.c
	@echo 'Building file: $<'
	@echo 'Executing target #15 $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	arm-none-eabi-gcc "@freertos/queue.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "freertos/queue.o" "$<"
	@echo 'Finished building: $<'
	@echo ' '

freertos/tasks.o: ../freertos/tasks.c
	@echo 'Building file: $<'
	@echo 'Executing target #16 $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	arm-none-eabi-gcc "@freertos/tasks.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "freertos/tasks.o" "$<"
	@echo 'Finished building: $<'
	@echo ' '

freertos/timers.o: ../freertos/timers.c
	@echo 'Building file: $<'
	@echo 'Executing target #17 $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	arm-none-eabi-gcc "@freertos/timers.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "freertos/timers.o" "$<"
	@echo 'Finished building: $<'
	@echo ' '


