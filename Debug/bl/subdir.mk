################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.local

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS_QUOTED += \
"../bl/FlexCAN.c" \
"../bl/LPUART.c" \
"../bl/SPI_MSD0_Driver.c" \
"../bl/bl.c" \
"../bl/clocks_and_modes.c" 

C_SRCS += \
../bl/FlexCAN.c \
../bl/LPUART.c \
../bl/SPI_MSD0_Driver.c \
../bl/bl.c \
../bl/clocks_and_modes.c 

C_DEPS_QUOTED += \
"./bl/FlexCAN.d" \
"./bl/LPUART.d" \
"./bl/SPI_MSD0_Driver.d" \
"./bl/bl.d" \
"./bl/clocks_and_modes.d" 

OBJS_QUOTED += \
"./bl/FlexCAN.o" \
"./bl/LPUART.o" \
"./bl/SPI_MSD0_Driver.o" \
"./bl/bl.o" \
"./bl/clocks_and_modes.o" 

C_DEPS += \
./bl/FlexCAN.d \
./bl/LPUART.d \
./bl/SPI_MSD0_Driver.d \
./bl/bl.d \
./bl/clocks_and_modes.d 

OBJS_OS_FORMAT += \
./bl/FlexCAN.o \
./bl/LPUART.o \
./bl/SPI_MSD0_Driver.o \
./bl/bl.o \
./bl/clocks_and_modes.o 

OBJS += \
./bl/FlexCAN.o \
./bl/LPUART.o \
./bl/SPI_MSD0_Driver.o \
./bl/bl.o \
./bl/clocks_and_modes.o 


# Each subdirectory must supply rules for building sources it contributes
bl/FlexCAN.o: ../bl/FlexCAN.c
	@echo 'Building file: $<'
	@echo 'Executing target #12 $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	arm-none-eabi-gcc "@bl/FlexCAN.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "bl/FlexCAN.o" "$<"
	@echo 'Finished building: $<'
	@echo ' '

bl/LPUART.o: ../bl/LPUART.c
	@echo 'Building file: $<'
	@echo 'Executing target #13 $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	arm-none-eabi-gcc "@bl/LPUART.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "bl/LPUART.o" "$<"
	@echo 'Finished building: $<'
	@echo ' '

bl/SPI_MSD0_Driver.o: ../bl/SPI_MSD0_Driver.c
	@echo 'Building file: $<'
	@echo 'Executing target #14 $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	arm-none-eabi-gcc "@bl/SPI_MSD0_Driver.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "bl/SPI_MSD0_Driver.o" "$<"
	@echo 'Finished building: $<'
	@echo ' '

bl/bl.o: ../bl/bl.c
	@echo 'Building file: $<'
	@echo 'Executing target #15 $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	arm-none-eabi-gcc "@bl/bl.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "bl/bl.o" "$<"
	@echo 'Finished building: $<'
	@echo ' '

bl/clocks_and_modes.o: ../bl/clocks_and_modes.c
	@echo 'Building file: $<'
	@echo 'Executing target #16 $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	arm-none-eabi-gcc "@bl/clocks_and_modes.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "bl/clocks_and_modes.o" "$<"
	@echo 'Finished building: $<'
	@echo ' '


