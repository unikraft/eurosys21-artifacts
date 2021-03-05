#!/bin/bash
RAW="rhea-kvm_read.txt"
CSV="rhea-kvm_read.csv"
echo "RUN ALL (9pfs read)... This can take some time..."
echo "--------------------------------------------------"
echo "Running 9pfs read with 4K..."
./runqemu.sh randomfile 4 &> $RAW
echo "Running 9pfs read with 8K..."
./runqemu.sh randomfile 8 &>> $RAW
echo "Running 9pfs read with 16K..."
./runqemu.sh randomfile 16 &>> $RAW
echo "Running 9pfs read with 32K..."
./runqemu.sh randomfile 32 &>> $RAW
echo "Running 9pfs read with 64K..."
./runqemu.sh randomfile 64 &>> $RAW
echo "Raw results are saved into $RAW"
../9pfs_csv.sh $RAW > $CSV
echo "CSV results are saved into $CSV"