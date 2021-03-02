#!/bin/bash
#Initializing the setup for 9pfs latency
CURRENT_FOLDER=$PWD
echo "Download the VM from remote server (this will take some times...)"
curl -LO https://people.montefiore.uliege.be/gain/unikraft/vm.zip

echo "Unzip the VM..."
unzip vm.zip

echo "Go to the VM folder and generate a random file for the reading"
cd vm/test
head -c 1G </dev/urandom > randomfile
cd $CURRENT_FOLDER

echo "Download unikraft from git and checkout to specific commit"
git clone git@github.com:unikraft/unikraft.git
cd unikraft
git checkout 7fd6797bd5917acc515ef6ddbfa85621f4aacf5f

echo "Generate random file for the unikraft read"
cd "$CURRENT_FOLDER/apps/lib-readfile/"
mkdir test
head -c 1G </dev/urandom > test/randomfile

echo "Compiling lib-readfile..."
make

echo "Compiling lib-writefile..."
cd "$CURRENT_FOLDER/apps/lib-writefile/"
mkdir test
make

cd $CURRENT_FOLDER
echo "All is setup. Please refer to the README.md for the test"
