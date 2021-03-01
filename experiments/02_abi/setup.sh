#!/bin/bash
#Initializing the setup for abi compatibility
echo "Go to /tmp folder"
cp README.md /tmp
cd /tmp
echo "Download the experiments archive from remote server"
wget https://people.montefiore.uliege.be/gain/unikraft/table2-experiments.zip
echo "Unzip the experiments..."
unzip table2-experiments.zip
echo "All is setup. Please refer to the README.md for the tests"
