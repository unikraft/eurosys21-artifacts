#!/bin/bash
rm *.csv &> /dev/null
for d in *; do
     if [[ -d $d ]]; then
        echo "Enter to $d repository"
        cd $d
        echo "Cleaning $d ..."
        make properclean &> "make_clean_$d.txt" 
        rm *.txt &> /dev/null
        cd ..
        echo "---------------------------------------------"
    fi
 done
