#!/bin/bash
CURRENT_FOLDER=$PWD

echo "Copy clean script in the abi folder"
cp script_abi/script_clean.sh abi/apps_musl/
cp script_abi/script_clean.sh abi/apps_newlib/

echo "Cleaning musl folders"
cd $CURRENT_FOLDER/abi/apps_musl/
./script_clean.sh
rm script_clean.sh

echo "Cleaning newlib folders"
cd $CURRENT_FOLDER/abi/apps_newlib/
./script_clean.sh
rm script_clean.sh

cd $CURRENT_FOLDER
