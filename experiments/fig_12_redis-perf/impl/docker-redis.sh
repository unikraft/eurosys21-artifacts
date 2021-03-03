#!/bin/bash

# verbose output
set -x

source ../common/set-cpus.sh
source ../common/network.sh
source ../common/redis.sh

IMAGES=$(pwd)/images

LOG=rawdata/docker-redis.txt
RESULTS=results/docker.csv
echo "operation	throughput" > $RESULTS

mkdir -p rawdata
touch $LOG

CONTAINER=redis-reqs-bench

function cleanup {
	# kill all children (evil)
        docker container stop $CONTAINER
        docker rm -f $CONTAINER
	pkill -P $$
}

docker pull redis:5.0.4-alpine

trap "cleanup" EXIT

for j in {1..5}
do
	killall -9 redis-server

	# one core, like other measurements
	# 5.0.4, same version (or similar) to other measurements
	docker run --rm -d --cpuset-cpus ${CPU2} --cpus=1 \
			--privileged --net=host --name=${CONTAINER} \
			-v $(pwd)/data:/usr/local/etc/redis \
			redis:5.0.4-alpine redis-server \
			/usr/local/etc/redis/redis.conf

	# make sure that the server has properly started
	sleep 2

	# benchmark
	benchmark_redis_server localhost 6379

	parse_redis_results $LOG $RESULTS

	# stop server
        docker container stop $CONTAINER
        docker rm -f $CONTAINER
	sleep 2
done
