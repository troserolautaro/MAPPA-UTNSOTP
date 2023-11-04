#!/bin/bash
#script de Utils 
cd Debug
echo 'compilando Utils'
gcc -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"src/utils.d" -MT"src/utils.o" -o "src/utils.o" "../src/utils.c"
gcc -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"src/utilsCliente.d" -MT"src/utilsCliente.o" -o "src/utilsCliente.o" "../src/utilsCliente.c"
gcc -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"src/utilsServidor.d" -MT"src/utilsServidor.o" -o "src/utilsServidor.o" "../src/utilsServidor.c"
echo 'compilando libUtils'
gcc -shared -o "libUtils.so" ./src/utils.o ./src/utilsCliente.o ./src/utilsServidor.o   
echo 'instalando Utils'
sudo cp -u libUtils.so /usr/lib
sudo cp -u ../src/*.h /usr/include
echo 'se instalaron los utils'
cd ..
