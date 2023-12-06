#!/bin/bash
#script de kernel
#
echo 'compilando binario de Kernel'
cd Debug

echo 'Building file: ../src/Consola.c'
echo 'Invoking: GCC C Compiler'
gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"src/Consola.d" -MT"src/Consola.o" -o "src/Consola.o" "../src/Consola.c"
echo 'echo "Finished building: ../src/Consola.c'

echo 'Building file: ../src/Generales.c'
echo 'Invoking: GCC C Compiler'
gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"src/Generales.d" -MT"src/Generales.o" -o "src/Generales.o" "../src/Generales.c"
echo 'Finished building: ../src/Generales.c'
 
echo 'Building file: ../src/PlanificadorCorto.c'
echo 'Invoking: GCC C Compiler'
gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"src/PlanificadorCorto.d" -MT"src/PlanificadorCorto.o" -o "src/PlanificadorCorto.o" "../src/PlanificadorCorto.c"
echo 'Finished building: ../src/PlanificadorCorto.c'
 
echo 'Building file: ../src/PlanificadorLargo.c'
echo 'Invoking: GCC C Compiler'
gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"src/PlanificadorLargo.d" -MT"src/PlanificadorLargo.o" -o "src/PlanificadorLargo.o" "../src/PlanificadorLargo.c"
echo 'Finished building: ../src/PlanificadorLargo.c'

echo 'Building file: ../src/KernelMemoria.c'
echo 'Invoking: GCC C Compiler'
gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"src/KernelMemoria.d" -MT"src/KernelMemoria.o" -o "src/KernelMemoria.o" "../src/KernelMemoria.c"
echo 'Finished building: ../src/KernelMemoria.c'

echo 'Building file: ../src/Kernel.c'
echo 'Invoking: GCC C Compiler'
gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"src/Kernel.d" -MT"src/Kernel.o" -o "src/Kernel.o" "../src/Kernel.c"
echo 'Finished building: ../src/Kernel.c'

echo 'Building target: Kernel'
echo 'Invoking: GCC C Linker'
gcc -o "Kernel" ./src/Consola.o ./src/Generales.o ./src/Kernel.o ./src/PlanificadorCorto.o ./src/PlanificadorLargo.o ./src/KernelMemoria.o   -lcommons -lUtils -lreadline
echo 'Finished building target: Kernel'

cd ..
echo 'ejecutando binario de Kernel'
valgrind -s ./Debug/Kernel