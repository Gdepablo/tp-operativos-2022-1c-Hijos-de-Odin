################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../algoritmos.c \
../com_kernel.c \
../hilo_cpu.c \
../hilo_kernel.c \
../main_memoria.c \
../swap.c 

OBJS += \
./algoritmos.o \
./com_kernel.o \
./hilo_cpu.o \
./hilo_kernel.o \
./main_memoria.o \
./swap.o 

C_DEPS += \
./algoritmos.d \
./com_kernel.d \
./hilo_cpu.d \
./hilo_kernel.d \
./main_memoria.d \
./swap.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -Icommons -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


