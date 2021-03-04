#!/bin/bash
#Use this script to generate the plot (after running experiment)
CURRENT_FOLDER=$PWD
DATA_FOLDER="data"

FILE1="$PWD/abi/apps_loc/cloc.csv"
FILE2="$PWD/abi/apps_musl_compat/musl-compat.csv"
FILE3="$PWD/abi/apps_newlib_compat/newlib-compat.csv"
FILE4="$PWD/abi/apps_musl_std/musl-std.csv"
FILE5="$PWD/abi/apps_newlib_std/newlib-std.csv"

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


if [ -f "$FILE4" ]; then
    cp "$FILE4" "$CURRENT_FOLDER/$DATA_FOLDER"
else
    echo "$FILE4 does not exist. Perform the abi experiment first"
    exit 1
fi


if [ -f "$FILE5" ]; then
    cp "$FILE5" "$CURRENT_FOLDER/$DATA_FOLDER"
else
    echo "$FILE5 does not exist. Perform the abi experiment first"
    exit 1
fi

python3 merge_csv.py
echo "Results are available in $PWD/merged.csv"
#cat merged.csv
