#!/bin/bash
rm *.csv &> /dev/null
for d in *; do
     if [[ -d $d ]]; then
        echo "Enter to $d repository"
        cd $d
        echo "Cleaning $d ... This may take some times"
        rm *.txt &> /dev/null
        make clean &> "make_clean_$d.txt" 
        cd ..
        echo "---------------------------------------------"
    fi
 done
