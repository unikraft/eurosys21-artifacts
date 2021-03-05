#!/bin/bash
RAW="rhea-kvm_write.txt"
CSV="rhea-kvm_write.csv"
echo "RUN ALL (9pfs write)... This can take a very long time..."
echo "--------------------------------------------------"
echo "Running 9pfs write with 4K..."
./runqemu.sh random4 4 &> $RAW
echo "Running 9pfs write with 8K..."
./runqemu.sh random8 8 &>> $RAW
echo "Running 9pfs write with 16K..."
./runqemu.sh random16 16 &>> $RAW
echo "Running 9pfs write with 32K..."
./runqemu.sh random32 32 &>> $RAW
echo "Running 9pfs write with 64K..."
./runqemu.sh random64 64 &>> $RAW
echo "Raw results are saved into $RAW"
../9pfs_csv.sh $RAW > $CSV
echo "CSV results are saved into $CSV"
rm test/random*