#!/bin/bash
CSV_FILE="$1.csv"
> "$CSV_FILE"
for d in *; do
     if [[ -d $d ]]; then
        echo "Enter to $d repository"
        cd $d
    
        if [[ -f "build/app-std_kvm-x86_64" ]]; then 
            echo "$d -> Build without the compat. layer"
            echo "$d,V" >> "../$CSV_FILE"
        else
            echo "$d -> Cannot build without the compat. layer"
            echo "$d,X" >> "../$CSV_FILE"
        fi
        cd ..
        echo "---------------------------------------------"
    fi
 done
 echo "CSV file is saved into $CSV_FILE"
