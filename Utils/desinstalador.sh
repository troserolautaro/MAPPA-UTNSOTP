#!/bin/bash
#script desinstalador de Utils
echo 'desinstalando Utils'
sudo rm   /usr/lib/libUtils.so
sudo rm  /usr/include/utils.h
sudo rm  /usr/include/utilsCliente.h
sudo rm  /usr/include/utilsServidor.h
echo 'se desinstalaron los utils'