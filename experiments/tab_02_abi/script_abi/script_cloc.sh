#!/bin/bash
> cloc.csv
for d in *; do
     if [[ -d $d ]]; then
        echo "Enter to $d repository"
        cd $d
        echo "Glue code size $d:"
        ./cloc.sh $d
        ./cloc.sh $d >> ../cloc.csv
        cd ..
        echo "---------------------------------------------"
    fi
 done
 echo "CSV file is saved into cloc.csv"
