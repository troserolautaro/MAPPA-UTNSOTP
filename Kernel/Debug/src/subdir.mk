################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/Consola.c \
../src/Generales.c \
../src/Kernel.c \
../src/PlanificadorCorto.c \
../src/PlanificadorLargo.c 

C_DEPS += \
./src/Consola.d \
./src/Generales.d \
./src/Kernel.d \
./src/PlanificadorCorto.d \
./src/PlanificadorLargo.d 

OBJS += \
./src/Consola.o \
./src/Generales.o \
./src/Kernel.o \
./src/PlanificadorCorto.o \
./src/PlanificadorLargo.o 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/git/tp-2023-2c-GrupoX/Utils/src" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src

clean-src:
	-$(RM) ./src/Consola.d ./src/Consola.o ./src/Generales.d ./src/Generales.o ./src/Kernel.d ./src/Kernel.o ./src/PlanificadorCorto.d ./src/PlanificadorCorto.o ./src/PlanificadorLargo.d ./src/PlanificadorLargo.o

.PHONY: clean-src

