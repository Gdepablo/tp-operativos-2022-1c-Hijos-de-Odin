################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../kernel.c \
../planificadorDeCortoPlazo.c \
../planificadorDeLargoPlazo.c \
../planificadorDeMedianoPlazo.c 

OBJS += \
./kernel.o \
./planificadorDeCortoPlazo.o \
./planificadorDeLargoPlazo.o \
./planificadorDeMedianoPlazo.o 

C_DEPS += \
./kernel.d \
./planificadorDeCortoPlazo.d \
./planificadorDeLargoPlazo.d \
./planificadorDeMedianoPlazo.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -Ipthreads -Icommons -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

planificadorDeLargoPlazo.o: ../planificadorDeLargoPlazo.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -Ipthreads -Icommons -O2 -g -Wall -Wextra -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"planificadorDeLargoPlazo.d" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


