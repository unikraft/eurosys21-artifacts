#!/bin/bash

# makes sure to run host setup
../common/setup-host.sh

mkdir -p rawdata results

# run benchmarks
./impl/rump-qemu-nginx.sh
./impl/lupine-fc-nginx.sh
./impl/lupine-qemu-nginx.sh
./impl/osv-qemu-nginx.sh
./impl/microvm-fc-nginx.sh
./impl/microvm-qemu-nginx.sh
./impl/docker-nginx.sh
./impl/native-nginx.sh
./impl/unikraft-qemu-nginx.sh
./impl/mirage-solo5-httpreply.sh
