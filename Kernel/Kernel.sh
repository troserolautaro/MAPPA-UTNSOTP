#!/bin/bash
#script de kernel
#
#gcc -I"/home/utnso/git/tp-2023-2c-GrupoX/Utils/src" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"src/Kernel.d" -MT"src/Kernel.o" -o "src/Kernel.o" "../src/Kernel.c"
echo 'compilando binario de Kernel'
gcc -o "Debug/kernel" src/Kernel.c -lcommons -lUtils -lreadline
echo 'ejecutando binario de Kernel'
valgrind -s ./Debug/Kernel