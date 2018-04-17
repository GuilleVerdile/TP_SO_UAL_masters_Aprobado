################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Socket/Coordinador.c \
../Socket/ESI.c \
../Socket/FuncionesConexiones.c \
../Socket/Instancia.c \
../Socket/Planificador.c 

OBJS += \
./Socket/Coordinador.o \
./Socket/ESI.o \
./Socket/FuncionesConexiones.o \
./Socket/Instancia.o \
./Socket/Planificador.o 

C_DEPS += \
./Socket/Coordinador.d \
./Socket/ESI.d \
./Socket/FuncionesConexiones.d \
./Socket/Instancia.d \
./Socket/Planificador.d 


# Each subdirectory must supply rules for building sources it contributes
Socket/%.o: ../Socket/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


