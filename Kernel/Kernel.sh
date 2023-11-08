#!/bin/bash
#script de kernel
#
echo 'compilando binario de Kernel'
cd Debug
gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"src/Kernel.d" -MT"src/Kernel.o" -o "src/Kernel.o" "../src/Kernel.c"
gcc -o "Kernel" ./src/Consola.o ./src/Generales.o ./src/Kernel.o ./src/PlanificadorCorto.o ./src/PlanificadorLargo.o    -lcommons -lUtils -lreadline
echo 'ejecutando binario de Kernel'
cd ..
valgrind -s ./Debug/Kernel