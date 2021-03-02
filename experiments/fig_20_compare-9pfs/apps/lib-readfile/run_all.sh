#!/bin/bash
RAW="rhea-kvm_read.txt"
CSV="rhea-kvm_read.csv"
echo "RUN ALL (9pfs read)... This can take some times..."
./runqemu.sh randomfile 4 &> $RAW
./runqemu.sh randomfile 8 &>> $RAW
./runqemu.sh randomfile 16 &>> $RAW
./runqemu.sh randomfile 32 &>> $RAW
./runqemu.sh randomfile 64 &>> $RAW
echo "Raw results are saved into $RAW"
../9pfs_csv.sh $RAW > $CSV
echo "CSV results are saved into $CSV"