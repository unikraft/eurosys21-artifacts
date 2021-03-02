#!/bin/bash

# verbose output
set -x

source ../common/set-cpus.sh
source ../common/network.sh
source ../common/redis.sh

apt install -y redis-server

LOG=rawdata/native-redis.txt
RESULTS=results/native-redis.csv
echo "operation	throughput" > $RESULTS
mkdir -p rawdata
touch $LOG

function cleanup {
	# kill all children (evil)
	killall -9 redis-server
	pkill -P $$
}

trap "cleanup" EXIT

for j in {1..5}
do
	taskset -c ${CPU2} redis-server $(pwd)/data/redis.conf &

	# make sure that the server has properly started
	sleep 2

	# benchmark
	benchmark_redis_server localhost 6379

	parse_redis_results $LOG $RESULTS

	# stop server
	killall -9 redis-server
done
