#!/bin/bash
#script de CPU
#
echo 'compilando binario de CPU'
cd Debug
gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"src/CPU.d" -MT"src/CPU.o" -o "src/CPU.o" "../src/CPU.c"
gcc -o "CPU" ./src/CPU.o    -lcommons -lUtils -lreadline
cd ..
echo 'ejecutando binario de CPU'
valgrind -s ./Debug/CPU