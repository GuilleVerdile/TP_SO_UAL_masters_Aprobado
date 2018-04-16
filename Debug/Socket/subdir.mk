################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Socket/ClienteChat.c \
../Socket/Coordinador.c \
../Socket/CoordinadorMultiple.c \
../Socket/ESI.c \
../Socket/Planificador.c 

OBJS += \
./Socket/ClienteChat.o \
./Socket/Coordinador.o \
./Socket/CoordinadorMultiple.o \
./Socket/ESI.o \
./Socket/Planificador.o 

C_DEPS += \
./Socket/ClienteChat.d \
./Socket/Coordinador.d \
./Socket/CoordinadorMultiple.d \
./Socket/ESI.d \
./Socket/Planificador.d 


# Each subdirectory must supply rules for building sources it contributes
Socket/%.o: ../Socket/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


