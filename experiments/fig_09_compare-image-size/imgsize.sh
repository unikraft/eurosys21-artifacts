#!/bin/bash

mkdir -p results

#./impl/alpine-size.sh
#./impl/docker-size.sh
#./impl/hermitux-light-size.sh
echo "Running hermitux..."
./impl/hermitux-size.sh
echo "Running lupine..."
./impl/lupine-size.sh
echo "Running osv..."
./impl/osv-size.sh
echo "Running rump..."
./impl/rump-size.sh
echo "Running native..."
./impl/native-size.sh
