#!/bin/bash

# makes sure to run host setup
../common/setup-host.sh

# run benchmarks
#./impl/hermitux-uhyve-redis.sh
#./impl/rump-qemu-redis.sh
./impl/osv-qemu-redis.sh
exit 1
./impl/lupine-fc-redis.sh
./impl/lupine-qemu-redis.sh
./impl/microvm-fc-redis.sh
./impl/microvm-qemu-redis.sh
./impl/docker-redis.sh
./impl/native-redis.sh
