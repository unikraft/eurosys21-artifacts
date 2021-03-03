#!/bin/bash
#Use this script to generate the plot (after running experiment)
CURRENT_FOLDER=$PWD
DATA_FOLDER="data"

FILE1="$PWD/abi/apps_loc/cloc.csv"
FILE2="$PWD/abi/apps_musl/musl-abi.csv"
FILE3="$PWD/abi/apps_newlib/newlib-abi.csv"

mkdir "$DATA_FOLDER"

if [ -f "$FILE1" ]; then
    cp "$FILE1" "$CURRENT_FOLDER/$DATA_FOLDER"
else
    echo "$FILE1 does not exist. Perform the abi experiment first"
    exit 1
fi

if [ -f "$FILE2" ]; then
    cp "$FILE2" "$CURRENT_FOLDER/$DATA_FOLDER"
else
    echo "$FILE1 does not exist. Perform the abi experiment first"
    exit 1
fi

if [ -f "$FILE3" ]; then
    cp "$FILE3" "$CURRENT_FOLDER/$DATA_FOLDER"
else
    echo "$FILE3 does not exist. Perform the abi experiment first"
    exit 1
fi

python3 merge_csv.py
echo "Results are available in $PWD/merged.csv"
cat merged.csv
