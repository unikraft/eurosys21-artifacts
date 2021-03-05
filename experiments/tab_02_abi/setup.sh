#!/bin/bash
#Initializing the setup for abi compatibility

if [ ${PWD:0:4} == "/tmp" ]; then
    echo "Already in /tmp folder"
else
    echo "$PWD" > .init_folder.txt
    echo "Go to /tmp folder for the test"
    mkdir -p /tmp/abi
    cp -r * /tmp/abi
    mv .init_folder.txt /tmp/abi
    cd /tmp/abi
fi
CURRENT_FOLDER=$PWD
ABI_FOLDER=abi

if [ -d "$ABI_FOLDER" ]; then
    echo "Directory already exists so skip download process to gain time"
else
    echo "Download the archive from remote server (this will take some time...)"
    curl -LO http://releases.unikraft.org/.eurosys21/abi.zip

    if [ ! -f abi.zip ]; then
        echo "Download the archive from remote backup server (this will take some time...)"
        curl -LO https://people.montefiore.uliege.be/gain/unikraft/abi.zip
    fi

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
