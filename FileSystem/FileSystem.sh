#!/bin/bash
#script de Filesystem
#
echo 'compilando binario de Filesystem'
cd Debug
gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"src/FileSystem.d" -MT"src/FileSystem.o" -o "src/FileSystem.o" "../src/FileSystem.c"
gcc -o "FileSystem" ./src/FileSystem.o    -lcommons -lUtils -lreadline
cd ..
echo 'ejecutando binario de Filesystem'
valgrind -s ./Debug/FileSystem

