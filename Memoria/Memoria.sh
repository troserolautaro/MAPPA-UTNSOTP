#!/bin/bash
#script de Memoria
#
cd Debug
echo 'Building file: ../src/MemoriaUsuario.c'
echo 'Invoking: GCC C Compiler'
gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"src/MemoriaUsuario.d" -MT"src/MemoriaUsuario.o" -o "src/MemoriaUsuario.o" "../src/MemoriaUsuario.c"
echo 'echo "Finished building: ../src/MemoriaUsuario.c'

echo 'Building file: ../src/MemoriaInstruciones.c'
echo 'Invoking: GCC C Compiler'
gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"src/MemoriaInstruciones.d" -MT"src/MemoriaInstruciones.o" -o "src/MemoriaInstruciones.o" "../src/MemoriaInstruciones.c"
echo 'echo "Finished building: ../src/MemoriaInstruciones.c'

echo 'Building file: ../src/MemoriaInstruciones.c'
echo 'Invoking: GCC C Compiler'
gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"src/Memoria.d" -MT"src/Memoria.o" -o "src/Memoria.o" "../src/Memoria.c"
echo 'echo "Finished building: ../src/MemoriaInstruciones.c'

echo 'Building target: Memoria'
echo 'Invoking: GCC C Linker'
gcc -o "Memoria" ./src/Memoria.o ./src/MemoriaInstruciones.o ./src/MemoriaUsuario.o    -lcommons -lUtils -lreadline
echo 'Finished building target: Memoria'

cd ..
echo 'ejecutando binario de Memoria'
valgrind -s --tool=helgrind ./Debug/Memoria
