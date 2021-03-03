#!/bin/bash
CURRENT_FOLDER=$PWD

cd apps/lib-readfile/
chmod +x ./run_all.sh
./run_all.sh

cd $CURRENT_FOLDER
cd apps/lib-writefile/
chmod +x ./run_all.sh
./run_all.sh

cd $CURRENT_FOLDER
