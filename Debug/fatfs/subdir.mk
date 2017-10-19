################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.local

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS_QUOTED += \
"../fatfs/ccsbcs.c" \
"../fatfs/diskio.c" \
"../fatfs/ff.c" \
"../fatfs/unicode.c" 

C_SRCS += \
../fatfs/ccsbcs.c \
../fatfs/diskio.c \
../fatfs/ff.c \
../fatfs/unicode.c 

C_DEPS_QUOTED += \
"./fatfs/ccsbcs.d" \
"./fatfs/diskio.d" \
"./fatfs/ff.d" \
"./fatfs/unicode.d" 

OBJS_QUOTED += \
"./fatfs/ccsbcs.o" \
"./fatfs/diskio.o" \
"./fatfs/ff.o" \
"./fatfs/unicode.o" 

C_DEPS += \
./fatfs/ccsbcs.d \
./fatfs/diskio.d \
./fatfs/ff.d \
./fatfs/unicode.d 

OBJS_OS_FORMAT += \
./fatfs/ccsbcs.o \
./fatfs/diskio.o \
./fatfs/ff.o \
./fatfs/unicode.o 

OBJS += \
./fatfs/ccsbcs.o \
./fatfs/diskio.o \
./fatfs/ff.o \
./fatfs/unicode.o 


# Each subdirectory must supply rules for building sources it contributes
fatfs/ccsbcs.o: ../fatfs/ccsbcs.c
	@echo 'Building file: $<'
	@echo 'Executing target #22 $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	arm-none-eabi-gcc "@fatfs/ccsbcs.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "fatfs/ccsbcs.o" "$<"
	@echo 'Finished building: $<'
	@echo ' '

fatfs/diskio.o: ../fatfs/diskio.c
	@echo 'Building file: $<'
	@echo 'Executing target #23 $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	arm-none-eabi-gcc "@fatfs/diskio.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "fatfs/diskio.o" "$<"
	@echo 'Finished building: $<'
	@echo ' '

fatfs/ff.o: ../fatfs/ff.c
	@echo 'Building file: $<'
	@echo 'Executing target #24 $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	arm-none-eabi-gcc "@fatfs/ff.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "fatfs/ff.o" "$<"
	@echo 'Finished building: $<'
	@echo ' '

fatfs/unicode.o: ../fatfs/unicode.c
	@echo 'Building file: $<'
	@echo 'Executing target #25 $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	arm-none-eabi-gcc "@fatfs/unicode.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "fatfs/unicode.o" "$<"
	@echo 'Finished building: $<'
	@echo ' '


