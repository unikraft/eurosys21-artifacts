#!/bin/bash
for d in *; do
     if [[ -d $d ]]; then
        echo "Enter to $d repository"
        cd $d
     
        echo "Compiling $d ... This may take some times"
        make &> "make_output_$d.txt" 
        if [[ -f "build/app-compat_kvm-x86_64" ]]
            mv build/app-compat_kvm-x86_64 .
        fi
        if [[ -f "build/app-std_kvm-x86_64" ]]
            mv build/app-std_kvm-x86_64 .
        fi
        make properclean
        cd ..
        echo "---------------------------------------------"
    fi
 done
