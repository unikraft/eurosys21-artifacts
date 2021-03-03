#!/bin/bash
CSV_FILE="$1.csv"
> "$CSV_FILE"
for d in *; do
     if [[ -d $d ]]; then
        echo "Enter to $d repository"
        cd $d
     
        size=$(ls -lh build/app-compat_kvm-x86_64|awk '{print $5}')
        if [[ $size == *"K"* ]]; then
            size=$(sed 's/.\{1\}$//' <<< "$size")
            size="0.$size"
        fi
        if [[ $size == *"M"* ]]; then
            size=$(sed 's/.\{1\}$//' <<< "$size")
            size="$size"
        fi
        echo "Size of $d: $size" 
        echo "$d,$size" >> "../$CSV_FILE"
        cd ..
        echo "---------------------------------------------"
    fi
 done
 echo "CSV file is saved into $CSV_FILE"
