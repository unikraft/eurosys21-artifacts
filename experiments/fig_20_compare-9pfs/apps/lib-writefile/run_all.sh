#!/bin/bash
RAW="rhea-kvm_write.txt"
CSV="rhea-kvm_write.csv"
echo "RUN ALL (9pfs write)... This can take some times..."
./runqemu.sh random4 4 &> $RAW
./runqemu.sh random8 8 &>> $RAW
./runqemu.sh random16 16 &>> $RAW
./runqemu.sh random32 32 &>> $RAW
./runqemu.sh random64 64 &>> $RAW
echo "Raw results are saved into $RAW"
../9pfs_csv.sh $RAW > $CSV
echo "CSV results are saved into $CSV"
rm test/random*