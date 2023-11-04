#!/bin/bash
#script de CPU
#
#gcc -I"/home/utnso/git/tp-2023-2c-GrupoX/Utils/src" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"src/CPU.d" -MT"src/CPU.o" -o "src/CPU.o" "../src/CPU.c"
echo 'compilando binario de CPU'
gcc -o "Debug/CPU" src/CPU.c -lcommons -lUtils -lreadline
echo 'ejecutando binario de CPU'
valgrind -s ./Debug/CPU