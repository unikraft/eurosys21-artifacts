#!/bin/bash
#Initializing the setup for abi compatibility
echo "go to /tmp"
mkdir -p /tmp/abi
cp -r * /tmp/abi
cd /tmp/abi

CURRENT_FOLDER=$PWD
ABI_FOLDER=abi

if [ -d "$ABI_FOLDER" ]; then
    echo "Directory already exists so skip download process to gain time"
else
    echo "Download the archive from remote server (this will take some times...)"
    curl -LO https://people.montefiore.uliege.be/gain/unikraft/abi.zip

    echo "Unzip the archive..."
    unzip abi.zip
fi

echo "Copy prepare scripts in the abi folder"
cp script_abi/script_prepare.sh abi/apps_musl/
cp script_abi/script_prepare.sh abi/apps_newlib/

echo "Prepare musl environment"
cd $CURRENT_FOLDER/abi/apps_musl/
./script_prepare.sh
rm script_prepare.sh

echo "Prepare newlib environment"
cd $CURRENT_FOLDER/abi/apps_newlib/
./script_prepare.sh
rm script_prepare.sh

cd $CURRENT_FOLDER
