#!/bin/bash
#Initializing the setup for abi compatibility

CURRENT_FOLDER=$PWD
ABI_FOLDER=abi

echo "Copy scripts in the abi folder"
cp script_abi/script_cloc.sh abi/apps_loc/
cp script_abi/script_run.sh abi/apps_musl/
cp script_abi/script_run.sh abi/apps_newlib/

echo "Compute the LoC"
cd abi/apps_loc/
./script_cloc.sh
rm script_cloc.sh

echo "Compute the musl libs size"
cd $CURRENT_FOLDER/abi/apps_musl/ 
./script_run.sh musl-abi
rm script_run.sh

echo "Compute the newlib libs size"
cd $CURRENT_FOLDER/abi/apps_newlib/
./script_run.sh newlib-abi
rm script_run.sh

cd $CURRENT_FOLDER
