################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../cpu.c \
../funcionesSocketsyConfig.c \
../instrucciones.c 

OBJS += \
./cpu.o \
./funcionesSocketsyConfig.o \
./instrucciones.o 

C_DEPS += \
./cpu.d \
./funcionesSocketsyConfig.d \
./instrucciones.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -Icommons -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


