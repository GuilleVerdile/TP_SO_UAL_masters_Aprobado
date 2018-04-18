################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Instancia/FuncionesConexiones.c \
../Instancia/Instancia.c 

OBJS += \
./Instancia/FuncionesConexiones.o \
./Instancia/Instancia.o 

C_DEPS += \
./Instancia/FuncionesConexiones.d \
./Instancia/Instancia.d 


# Each subdirectory must supply rules for building sources it contributes
Instancia/%.o: ../Instancia/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


