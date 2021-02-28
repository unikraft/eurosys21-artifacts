#!/bin/bash
#Execute these tests 20times
./runqemu.sh random4 4 &> 9pfs.txt
./runqemu.sh random8 8 &>> 9pfs.txt
./runqemu.sh random16 16 &>> 9pfs.txt
./runqemu.sh random32 32 &>> 9pfs.txt
./runqemu.sh random64 64 &>> 9pfs.txt
cat 9pfs.txt
ls test/
rm test/random*
