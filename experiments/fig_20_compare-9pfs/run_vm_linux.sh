#!/bin/bash
cd vm
echo "compile the program from sources"
chmod +x compile.sh
./compile.sh
clear
chmod +x launcher.sh
./launcher.sh
