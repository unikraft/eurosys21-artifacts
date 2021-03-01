#!/bin/bash

# makes sure to run host setup
../common/setup-host.sh

# run benchmarks
./impl/lupine-fc-nginx.sh
./impl/lupine-qemu-nginx.sh
./impl/osv-qemu-nginx.sh
./impl/microvm-fc-nginx.sh
./impl/microvm-qemu-nginx.sh
./impl/docker-nginx.sh
./impl/native-nginx.sh