#!/bin/bash
./runqemu.sh randomfile 4 &> 9pfs.txt
./runqemu.sh randomfile 8 &>> 9pfs.txt
./runqemu.sh randomfile 16 &>> 9pfs.txt
./runqemu.sh randomfile 32 &>> 9pfs.txt
./runqemu.sh randomfile 64 &>> 9pfs.txt
cat 9pfs.txt
