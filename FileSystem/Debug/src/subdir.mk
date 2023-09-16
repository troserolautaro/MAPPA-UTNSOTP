################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/FileSystem.c \
../src/utils_Cliente.c \
../src/utils_Servidor.c 

C_DEPS += \
./src/FileSystem.d \
./src/utils_Cliente.d \
./src/utils_Servidor.d 

OBJS += \
./src/FileSystem.o \
./src/utils_Cliente.o \
./src/utils_Servidor.o 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src

clean-src:
	-$(RM) ./src/FileSystem.d ./src/FileSystem.o ./src/utils_Cliente.d ./src/utils_Cliente.o ./src/utils_Servidor.d ./src/utils_Servidor.o

.PHONY: clean-src

