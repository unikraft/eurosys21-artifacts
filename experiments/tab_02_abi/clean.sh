#!/bin/bash
INIT_FOLDER=$PWD
if [ ${PWD:0:4} == "/tmp"  ]; then
    echo "Already in /tmp folder"
else
    cd /tmp/abi
fi

CURRENT_FOLDER=$PWD

echo "Copy clean script in the abi folder"
cp script_abi/script_clean.sh abi/apps_musl_compat/
cp script_abi/script_clean.sh abi/apps_musl_std/
cp script_abi/script_clean.sh abi/apps_newlib_std/
cp script_abi/script_clean.sh abi/apps_newlib_compat/

echo "Cleaning musl folders"
cd $CURRENT_FOLDER/abi/apps_musl_compat/
./script_clean.sh
rm script_clean.sh
cd $CURRENT_FOLDER/abi/apps_musl_std/
./script_clean.sh
rm script_clean.sh

echo "Cleaning newlib folders"
cd $CURRENT_FOLDER/abi/apps_newlib_compat/
./script_clean.sh
rm script_clean.sh
cd $CURRENT_FOLDER/abi/apps_newlib_std/
./script_clean.sh
rm script_clean.sh

cd $INIT_FOLDER
