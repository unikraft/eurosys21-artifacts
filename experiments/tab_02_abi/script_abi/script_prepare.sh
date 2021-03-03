#!/bin/bash
for d in *; do
     if [[ -d $d ]]; then
        echo "Enter to $d repository"
        cd $d
     
        echo "Compiling $d ... This may take some times"
        make &> "make_output_$d.txt" 
        cd ..
        echo "---------------------------------------------"
    fi
 done
