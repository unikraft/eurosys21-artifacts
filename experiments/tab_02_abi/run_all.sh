#!/bin/bash
#Running the experiment

if [ ! -d "/tmp/abi" ]; then
    echo "/tmp/abi does not exist. Run 'make prepare' first"
    exit 1
fi


if [ ${PWD:0:4} == "/tmp" ]; then
    echo "Already in /tmp folder"
else
    echo "Go to /tmp folder for the test"
    cd /tmp/abi
fi


CURRENT_FOLDER=$PWD
ABI_FOLDER=abi

echo "Copy scripts in the abi folder"
cp script_abi/script_cloc.sh abi/apps_loc/
cp script_abi/script_run.sh abi/apps_musl_compat/
cp script_abi/script_run.sh abi/apps_newlib_compat/
cp script_abi/script_check.sh abi/apps_musl_std/
cp script_abi/script_check.sh abi/apps_newlib_std/


echo "Compute the LoC"
cd abi/apps_loc/
./script_cloc.sh
rm script_cloc.sh

echo "Compute the musl compat libs size"
cd $CURRENT_FOLDER/abi/apps_musl_compat/
./script_run.sh musl-compat
rm script_run.sh

echo "Compute the newlib compat libs size"
cd $CURRENT_FOLDER/abi/apps_newlib_compat/
./script_run.sh newlib-compat
rm script_run.sh


echo "Check the musl std libs"
cd $CURRENT_FOLDER/abi/apps_musl_std/ 
./script_check.sh musl-std
rm script_check.sh

echo "Check the newlib std libs"
cd $CURRENT_FOLDER/abi/apps_newlib_std/
./script_check.sh newlib-std
rm script_check.sh

echo "All results are saved in /tmp/abi, please run 'merge.sh' to generate the csv table and copy results in the base folder"
