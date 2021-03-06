#!/bin/bash
#Use this script to generate the plot (after running experiment)

if [ ! -d "/tmp/abi" ]; then
    echo "/tmp/abi does not exist. Run 'make prepare' first"
    exit 1
fi

if [ ${PWD:0:4} == "/tmp" ]; then
    echo "Already in /tmp folder"
else
    echo "Go to /tmp folder"
    cd /tmp/abi
fi

CURRENT_FOLDER=$PWD
RESULT_FOLDER="results"

FILE1="$PWD/abi/apps_loc/cloc.csv"
FILE2="$PWD/abi/apps_musl_compat/musl-compat.csv"
FILE3="$PWD/abi/apps_newlib_compat/newlib-compat.csv"
FILE4="$PWD/abi/apps_musl_std/musl-std.csv"
FILE5="$PWD/abi/apps_newlib_std/newlib-std.csv"

mkdir -p "$RESULT_FOLDER"

if [ -f "$FILE1" ]; then
    cp "$FILE1" "$CURRENT_FOLDER/$RESULT_FOLDER"
else
    echo "$FILE1 does not exist. Perform the abi experiment first"
    exit 1
fi

if [ -f "$FILE2" ]; then
    cp "$FILE2" "$CURRENT_FOLDER/$RESULT_FOLDER"
else
    echo "$FILE1 does not exist. Perform the abi experiment first"
    exit 1
fi

if [ -f "$FILE3" ]; then
    cp "$FILE3" "$CURRENT_FOLDER/$RESULT_FOLDER"
else
    echo "$FILE3 does not exist. Perform the abi experiment first"
    exit 1
fi


if [ -f "$FILE4" ]; then
    cp "$FILE4" "$CURRENT_FOLDER/$RESULT_FOLDER"
else
    echo "$FILE4 does not exist. Perform the abi experiment first"
    exit 1
fi


if [ -f "$FILE5" ]; then
    cp "$FILE5" "$CURRENT_FOLDER/$RESULT_FOLDER"
else
    echo "$FILE5 does not exist. Perform the abi experiment first"
    exit 1
fi

python3 merge_csv.py

echo "Copy results and move to base folder"
base_folder=$(cat .init_folder.txt)
cp $PWD/merged.csv $base_folder
cp -r $RESULT_FOLDER $base_folder

echo "Raw results are available in the results folder and CSV merged table is written in merged.csv"
