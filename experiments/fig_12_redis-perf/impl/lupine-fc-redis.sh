#!/bin/bash

# verbose output
set -x

NETIF=tap100
LOG=rawdata/lupine-fc-redis.txt
RESULTS=results/lupine-fc.csv
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
	rm ${IMAGES}/redis.ext2.disposible
	pkill -P $$
	delete_tap $NETIF
}

trap "cleanup" EXIT

create_tap $NETIF $BASEIP

for j in {1..5}
do
	cp ${IMAGES}/redis.ext2 ${IMAGES}/redis.ext2.disposible

	taskset -c 6,7 firectl --firecracker-binary=./.firecracker \
                --kernel ${IMAGES}/lupine-fc.kernel \
                --tap-device=tap100/AA:FC:00:00:00:01 \
                --root-drive=${IMAGES}/redis.ext2.disposible \
                --ncpus=1 --memory=1024 \
                --kernel-opts="console=ttyS0 panic=-1 pci=off tsc=reliable ipv6.disable=1 init=/guest_start.sh /trusted/redis-server" &

	# make sure that the server has properly started
	sleep 3

	# benchmark
	benchmark_redis_server ${BASEIP}.2 6379

	parse_redis_results $LOG $RESULTS

	# stop server
	killall -9 firecracker firectl .firecracker
	rm ${IMAGES}/redis.ext2.disposible
done
