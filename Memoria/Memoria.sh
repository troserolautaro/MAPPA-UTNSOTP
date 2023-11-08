#!/bin/bash
#script de Memoria
#
cd Debug
echo 'compilando binario de Memoria'
gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"src/Memoria.d" -MT"src/Memoria.o" -o "src/Memoria.o" "../src/Memoria.c"
gcc -o "Memoria" ./src/Memoria.o ./src/MemoriaInstruciones.o ./src/MemoriaUsuario.o    -lcommons -lUtils -lreadline
cd ..
echo 'ejecutando binario de Memoria'
valgrind -s ./Debug/Memoria