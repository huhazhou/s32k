################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.local

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS_QUOTED += \
"../src/convert.c" \
"../src/main.c" 

C_SRCS += \
../src/convert.c \
../src/main.c 

C_DEPS_QUOTED += \
"./src/convert.d" \
"./src/main.d" 

OBJS_QUOTED += \
"./src/convert.o" \
"./src/main.o" 

C_DEPS += \
./src/convert.d \
./src/main.d 

OBJS_OS_FORMAT += \
./src/convert.o \
./src/main.o 

OBJS += \
./src/convert.o \
./src/main.o 


# Each subdirectory must supply rules for building sources it contributes
src/convert.o: ../src/convert.c
	@echo 'Building file: $<'
	@echo 'Executing target #21 $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	arm-none-eabi-gcc "@src/convert.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "src/convert.o" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/main.o: ../src/main.c
	@echo 'Building file: $<'
	@echo 'Executing target #22 $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	arm-none-eabi-gcc "@src/main.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "src/main.o" "$<"
	@echo 'Finished building: $<'
	@echo ' '


