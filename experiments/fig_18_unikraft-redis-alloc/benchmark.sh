#!/bin/bash

# verbose output
set -x

source ../common/set-cpus.sh
source ../common/network.sh
source ../common/redis.sh
source ../common/qemu.sh

IMAGES=$(pwd)/images/
BASEIP=172.190.0
NETIF=unikraft0
mkdir -p rawdata results

create_bridge $NETIF $BASEIP

kill_qemu
dnsmasq_pid=$(run_dhcp $NETIF $BASEIP)

function cleanup {
	# kill all children (evil)
	kill_dhcp $dnsmasq_pid
	kill_qemu
	pkill -P $$
	delete_bridge $NETIF
}

trap "cleanup" EXIT

for alloc in "buddy" "tinyalloc" "mimalloc" "tlsf"
do
	RESULTS=results/${alloc}.csv
	echo "operation	throughput" > $RESULTS

	LOG=rawdata/${alloc}.txt
	touch $LOG

	for j in {1..5}
	do
		taskset -c ${CPU1} qemu-guest \
			-i redis.cpio \
			-k ${IMAGES}/unikraft+${alloc}.kernel \
			-a "/redis.conf" -m 1024 -p ${CPU2} \
			-b ${NETIF} -x

		# make sure that the server has properly started
		sleep 8

		ip=`cat $(pwd)/dnsmasq.log | \
			grep "dnsmasq-dhcp: DHCPACK(${NETIF})" | \
			tail -n 1 | awk  '{print $3}'`

		# benchmark
		benchmark_redis_server ${ip} 6379

		# stop server
		kill_qemu
	done

	getop=`cat $LOG | tr ",\"" " " | awk -e '$0 ~ /GET/ {print $2}' | \
		sed -r '/^\s*$/d' | \
		awk '{ total += $1; count++ } END { OFMT="%f"; print total/(count*1000) }'`
	echo "GET	${getop}" >> $RESULTS

	setop=`cat $LOG | tr ",\"" " " | awk -e '$0 ~ /SET/ {print $2}' | \
		sed -r '/^\s*$/d' | \
		awk '{ total += $1; count++ } END { OFMT="%f"; print total/(count*1000) }'`
	echo "SET	${setop}" >> $RESULTS
done
