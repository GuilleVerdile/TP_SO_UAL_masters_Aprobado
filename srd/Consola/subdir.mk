################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Consola/Consola.c 

O_SRCS += \
../Consola/Consola.o 

OBJS += \
./Consola/Consola.o 

C_DEPS += \
./Consola/Consola.d 


# Each subdirectory must supply rules for building sources it contributes
Consola/%.o: ../Consola/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


