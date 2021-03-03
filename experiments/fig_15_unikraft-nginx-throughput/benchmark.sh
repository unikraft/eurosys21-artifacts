#!/bin/bash

# verbose output
set -x

source ../common/set-cpus.sh
source ../common/nginx.sh
source ../common/network.sh

IMAGES=images/
BASEIP=172.190.0
NETIF=unikraft0
mkdir -p rawdata results

create_bridge $NETIF $BASEIP
kill_qemu

# run dnsmasq
dnsmasq_pid=$(run_dhcp $NETIF $BASEIP)

function cleanup {
	# kill all children (evil)
	kill_dhcp $dnsmasq_pid
	delete_bridge $NETIF
	kill_qemu
	pkill -P $$
}

trap "cleanup" EXIT

for alloc in "buddy" "tinyalloc" "mimalloc" "tlsf"
do
	RESULTS=results/${alloc}.csv
	echo "throughput_reqs" > $RESULTS

	LOG=rawdata/${alloc}.txt
	touch $LOG

	for j in {1..5}
	do
		taskset -c ${CPU1} qemu-guest \
			-i nginx.cpio \
			-k ${IMAGES}/unikraft+${alloc}.kernel \
			-a "" -m 1024 -p ${CPU2} \
			-b ${NETIF} -x

		child_pid=$!

		# make sure that the server has properly started
		sleep 5

		ip=`cat $(pwd)/dnsmasq.log | \
			grep "dnsmasq-dhcp: DHCPACK(${NETIF})" | \
			tail -n 1 | awk  '{print $3}'`

		# benchmark
		benchmark_nginx_server ${ip} $LOG
		#curl http://${BASEIP}.2/index.html --noproxy ${BASEIP}.2 --output -

		# stop server
		kill -9 $child_pid
		kill_qemu
	done

	parse_nginx_results $LOG $RESULTS
done
