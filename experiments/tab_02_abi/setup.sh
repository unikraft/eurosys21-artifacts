#!/bin/bash
#Initializing the setup for abi compatibility
CURRENT_FOLDER=$PWD
echo "Go to /tmp folder"
cd /tmp
echo "Download the experiments archive from remote server"
wget https://people.montefiore.uliege.be/gain/unikraft/table2-experiments.zip
echo "Unzip the experiments..."
unzip table2-experiments.zip
cd abi
cp "$CURRENT_FOLDER/README.md" .
echo "All is setup. Please refer to the README.md for the tests. Be sure that you are in the /tmp/abi folder"
cd /tmp