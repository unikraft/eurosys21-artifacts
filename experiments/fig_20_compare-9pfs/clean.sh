#!/bin/bash
CURRENT_FOLDER=$PWD
echo "Removing VM folder"
rm -rf vm

echo "Cleaning lib-writefile"
cd apps/lib-writefile/
rm -rf test/
rm *.csv
rm *.txt
make properclean

cd $CURRENT_FOLDER
echo "Cleaning lib-readfile"
cd apps/lib-readfile/
rm -rf test/
rm *.csv
rm *.txt
make properclean

cd $CURRENT_FOLDER
echo "Removing unikraft folder"
rm -rf unikraft
