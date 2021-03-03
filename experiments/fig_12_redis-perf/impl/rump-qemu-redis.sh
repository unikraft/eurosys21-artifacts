#!/bin/bash

# verbose output
set -x

IMAGES=$(pwd)/images

source ../common/set-cpus.sh
source ../common/network.sh
source ../common/redis.sh
source ../common/qemu.sh

NETIF=tap10
LOG=rawdata/rump-qemu-redis.txt
RESULTS=results/rump-qemu.csv
echo "operation	throughput" > $RESULTS
mkdir -p rawdata
touch $LOG

kill_qemu

create_tap $NETIF $BASEIP

dnsmasq_pid=$(run_dhcp $NETIF $BASEIP)

function cleanup {
	# kill all children (evil)
	delete_tap $NETIF 
	kill_dhcp $dnsmasq_pid
	rm ${IMAGES}/rump-qemu.img.disposible
	kill_qemu
	pkill -P $$
}

trap "cleanup" EXIT

for j in {1..5}
do
	cp ${IMAGES}/rump-qemu.img ${IMAGES}/rump-qemu.img.disposible

	taskset -c ${CPU1},${CPU2} ${IMAGES}/rump/root/rumprun/app-tools/rumprun \
			kvm -i -M 1024 -g '-daemonize' \
        		-I if,vioif,"-net tap,script=no,ifname=$NETIF" \
        		-W if,inet,dhcp \
			-b ${IMAGES}/rump-qemu.iso,/data \
			-- ${IMAGES}/rump-qemu.img /data/conf/redis.conf

	# make sure that the server has properly started
	sleep 15

	ip=`cat $(pwd)/dnsmasq.log | grep "dnsmasq-dhcp: DHCPACK(${NETIF})" | \
				     tail -n 1 | awk  '{print $3}'`

	# benchmark
	benchmark_redis_server ${ip} 6379

	parse_redis_results $LOG $RESULTS

	# stop server
	killall -9 qemu-system-x86
	pkill -9 qemu-system-x86
	rm ${IMAGES}/rump-qemu.img.disposible
done
