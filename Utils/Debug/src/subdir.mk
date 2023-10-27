################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/utils.c \
../src/utilsCliente.c \
../src/utilsServidor.c 

C_DEPS += \
./src/utils.d \
./src/utilsCliente.d \
./src/utilsServidor.d 

OBJS += \
./src/utils.o \
./src/utilsCliente.o \
./src/utilsServidor.o 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src

clean-src:
	-$(RM) ./src/utils.d ./src/utils.o ./src/utilsCliente.d ./src/utilsCliente.o ./src/utilsServidor.d ./src/utilsServidor.o

.PHONY: clean-src

