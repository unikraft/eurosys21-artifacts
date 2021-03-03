#!/bin/bash
CURRENT_FOLDER=$PWD
echo "Cleaning VM folder"
rm -rf vm/test &> /dev/null
#rm -rf vm #TODO if necessary

echo "Cleaning lib-writefile"
cd apps/lib-writefile/
rm -rf test/ &> /dev/null
rm *.csv &> /dev/null
rm *.txt &> /dev/null
make properclean

cd $CURRENT_FOLDER
echo "Cleaning lib-readfile"
cd apps/lib-readfile/
rm -rf test/ &> /dev/null
rm *.csv &> /dev/null
rm *.txt &> /dev/null
make properclean

cd $CURRENT_FOLDER
echo "Removing unikraft folder"
rm -rf unikraft
