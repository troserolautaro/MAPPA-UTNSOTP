#!/bin/bash
#script de Utils
echo 'instalando Utils'
sudo cp -u Debug/libUtils.so /usr/lib
sudo cp -u src/*.h /usr/include
echo 'se instalaron los utils'