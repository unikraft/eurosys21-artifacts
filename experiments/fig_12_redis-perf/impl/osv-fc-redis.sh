#!/bin/bash

# verbose output
set -x

NETIF=osv0
LOG=rawdata/osv-fc-redis.txt
RESULTS=results/osv-fc.csv
echo "operation	throughput" > $RESULTS

IMAGES=$(pwd)/images

mkdir -p rawdata
touch $LOG

source ../common/set-cpus.sh
source ../common/network.sh
source ../common/redis.sh

killall -9 firecracker firectl .firecracker firecracker-x86
create_bridge $NETIF $BASEIP
dnsmasq_pid=$(run_dhcp $NETIF $BASEIP)

function cleanup {
	# kill all children (evil)
	killall -9 firecracker firectl .firecracker firecracker-x86
	rm ${IMAGES}/osv-qemu.img.disposible
	pkill -P $$
	kill_dhcp $dnsmasq_pid
	delete_bridge $NETIF
}

trap "cleanup" EXIT

for j in {1..5}
do
	cp ${IMAGES}/osv-qemu.img ${IMAGES}/osv-qemu.img.disposible

	pushd ${IMAGES}/osv/root/osv/scripts
	taskset -c 6,7 ./firecracker.py -n \
                -k ${IMAGES}/osv-fc.kernel \
                -b $NETIF --vcpus=1 -m 1G \
                -i ${IMAGES}/osv-qemu.img.disposible &
	popd

	child_pid=$!

	# make sure that the server has properly started
	sleep 3

	ip=`cat $(pwd)/dnsmasq.log | grep "dnsmasq-dhcp: DHCPACK(${NETIF})" | \
			 tail -n 1 | awk  '{print $3}'`

	# benchmark
	benchmark_redis_server ${ip} 6379

	parse_redis_results $LOG $RESULTS

	# stop server
	killall -9 firecracker firectl .firecracker firecracker-x86
	kill -9 $child_pid
	kill $(ps -s $$ -o pid=)
	rm ${IMAGES}/osv-qemu.img.disposible
done
