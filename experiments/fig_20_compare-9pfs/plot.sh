#!/bin/bash
#Use this script to generate the plot (after running experiment)
CURRENT_FOLDER=$(dirname $0)
RESULT_FOLDER="results"
FILE1="$PWD/apps/lib-readfile/unikraft-kvm_read.csv"
FILE2="$PWD/apps/lib-writefile/unikraft-kvm_write.csv"
FILE3="$PWD/vm/test/linux-kvm_read.csv"
FILE4="$PWD/vm/test/linux-kvm_write.csv"


mkdir -p "$RESULT_FOLDER"

if [ -f "$FILE1" ]; then
    cp "$FILE1" "$CURRENT_FOLDER/$RESULT_FOLDER"
else 
    echo "$FILE1 does not exist. Perform the unikraft 9pfs experiment first"
    exit 1
fi

if [ -f "$FILE2" ]; then
    cp "$FILE2" "$CURRENT_FOLDER/$RESULT_FOLDER"
else 
    echo "$FILE1 does not exist. Perform the unikraft 9pfs experiment first"
    exit 1
fi

if [ -f "$FILE3" ]; then
    cp "$FILE3" "$CURRENT_FOLDER/$RESULT_FOLDER"
else 
    echo "$FILE3 does not exist. Perform the VM experiment first"
    exit 1
fi

if [ -f "$FILE4" ]; then
    cp "$FILE4" "$CURRENT_FOLDER/$RESULT_FOLDER"
else 
    echo "$FILE4 does not exist. Perform the VM experiment first"
    exit 1
fi

python3 plot.py --data $RESULT_FOLDER --output fig_20_compare-9pfs.svg