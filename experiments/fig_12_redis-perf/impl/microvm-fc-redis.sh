#!/bin/bash

# verbose output
set -x

LOG=rawdata/microvm-fc-redis.txt
RESULTS=results/microvm-fc.csv
echo "operation	throughput" > $RESULTS
mkdir -p rawdata
touch $LOG

IMAGES=$(pwd)/images

source ../common/set-cpus.sh
source ../common/network.sh
source ../common/redis.sh

function cleanup {
	# kill all children (evil)
	killall -9 firecracker firectl cp .firecracker
	rm /tmp/redis.ext2.disposible
	pkill -P $$
	delete_tap tap100
}

trap "cleanup" EXIT

create_tap tap100 $BASEIP

for j in {1..5}
do
	cp ${IMAGES}/redis.ext2 /tmp/redis.ext2.disposible

	taskset -c ${CPU1},${CPU2} firectl --firecracker-binary=./.firecracker \
                --kernel ${IMAGES}/generic-fc.kernel \
                --tap-device=tap100/AA:FC:00:00:00:01 \
                --root-drive=/tmp/redis.ext2.disposible \
                --ncpus=1 --memory=1024 \
                --kernel-opts="console=ttyS0 panic=-1 pci=off tsc=reliable ipv6.disable=1 init=/guest_start.sh redis-server" &

	# make sure that the server has properly started
	sleep 3

	# benchmark
	benchmark_redis_server ${BASEIP}.2 6379

	parse_redis_results $LOG $RESULTS

	# stop server
	killall -9 firecracker firectl .firecracker
	rm /tmp/redis.ext2.disposible
done
