################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Planificador/Consola.c \
../Planificador/FuncionesConexiones.c \
../Planificador/Planificador.c 

O_SRCS += \
../Planificador/Consola.o \
../Planificador/FuncionesConexiones.o \
../Planificador/Planificador.o 

OBJS += \
./Planificador/Consola.o \
./Planificador/FuncionesConexiones.o \
./Planificador/Planificador.o 

C_DEPS += \
./Planificador/Consola.d \
./Planificador/FuncionesConexiones.d \
./Planificador/Planificador.d 


# Each subdirectory must supply rules for building sources it contributes
Planificador/%.o: ../Planificador/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


