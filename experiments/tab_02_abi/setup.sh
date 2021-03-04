#!/bin/bash
#Initializing the setup for abi compatibility
echo "$PWD" > .init_folder.txt

echo "go to /tmp folder for the test"
mkdir -p /tmp/abi
cp -r * /tmp/abi
cd /tmp/abi

CURRENT_FOLDER=$PWD
ABI_FOLDER=abi

if [ -d "$ABI_FOLDER" ]; then
    echo "Directory already exists so skip download process to gain time"
else
    echo "Download the archive from remote server (this will take some time...)"
    curl -LO https://people.montefiore.uliege.be/gain/unikraft/abi.zip

    echo "Unzip the archive..."
    unzip abi.zip
fi

echo "Copy prepare scripts in the right abi folders"
cp script_abi/script_prepare.sh abi/apps_musl_compat/
cp script_abi/script_prepare.sh abi/apps_musl_std/
cp script_abi/script_prepare.sh abi/apps_newlib_compat/
cp script_abi/script_prepare.sh abi/apps_newlib_std/

echo "Prepare the musl compat environment (this will take some time)"
cd $CURRENT_FOLDER/abi/apps_musl_compat/ 
./script_prepare.sh
rm script_prepare.sh

echo "Prepare the newlib compat environment (this will take some time)"
cd $CURRENT_FOLDER/abi/apps_newlib_compat/
./script_prepare.sh
rm script_prepare.sh

echo "Prepare the musl std environment (this will take some time)"
cd $CURRENT_FOLDER/abi/apps_musl_std/ 
./script_prepare.sh
rm script_prepare.sh

echo "Prepare the newlib std environment (this will take some time)"
cd $CURRENT_FOLDER/abi/apps_newlib_std/
./script_prepare.sh
rm script_prepare.sh

cd $CURRENT_FOLDER
