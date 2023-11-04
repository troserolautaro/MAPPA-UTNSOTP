#!/bin/bash
#script de Memoria
#
#gcc -I"/home/utnso/git/tp-2023-2c-GrupoX/Utils/src" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"src/Memoria.d" -MT"src/Memoria.o" -o "src/Memoria.o" "../src/Memoria.c"
echo 'compilando binario de Memoria'
gcc -o "Debug/Memoria" src/Memoria.c -lcommons -lUtils -lreadline
echo 'ejecutando binario de Memoria'
valgrind -s ./Debug/Memoria