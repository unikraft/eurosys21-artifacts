#!/bin/bash

# verbose output
set -x

IMAGES=$(pwd)/images/
BASEIP=172.190.0
NETIF=hermitux0
LOG=rawdata/hermitux-uhyve-redis.txt
RESULTS=results/hermitux-uhyve.csv
echo "operation	throughput" > $RESULTS
mkdir -p rawdata
touch $LOG

source ../common/set-cpus.sh
source ../common/network.sh
source ../common/redis.sh

# hermitux variables
VERBOSE=1
HERMITUX_PATH=${IMAGES}/hermitux/
PROXY=./uhyve
KERNEL=./hermitux.kernel
PROG=./redis-server
ARGS="redis.conf"
MINIFS=1
MINIFS_PATH=.minifs
PROF=0
GATEWAY=${BASEIP}.1
HERMITUX_IP=${BASEIP}.4

function cleanup {
	# kill all children (evil)
	pkill -P $$
	delete_tap $NETIF
}

trap "cleanup" EXIT

create_tap $NETIF $BASEIP

for j in {1..5}
do
	pushd $HERMITUX_PATH
	HERMIT_MEM=1G HERMIT_VERBOSE=$VERBOSE HERMIT_ISLE=uhyve HERMIT_TUX=1 \
		      HERMIT_MINIFS=$MINIFS HERMIT_MINIFS_HOSTLOAD=$MINIFS_PATH \
		      HERMIT_NETIF=$NETIF HERMIT_PROFILE=$PROF HERMIT_CPUS=1 \
		      HERMIT_IP=$HERMITUX_IP HERMIT_GATEWAY=$GATEWAY \
		      taskset -c ${CPU1},${CPU2} $PROXY $KERNEL $PROG $ARGS &
	popd

	child_pid=$!

	# make sure that the server has properly started
	sleep 6

	# benchmark, limit to 10 conns otherwise it sends RSTs
	CONCURRENT_CONNS=10
	benchmark_redis_server ${HERMITUX_IP} 8000

	parse_redis_results $LOG $RESULTS

	# stop server
	kill -9 $child_pid
	kill $(ps -s $$ -o pid=)
done
