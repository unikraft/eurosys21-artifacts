#!/bin/bash

# verbose output
set -x

source ../common/set-cpus.sh

IMAGES=images/
BASEIP=172.190.0
NETIF=unikraft0
mkdir -p rawdata results

echo "creating bridge"
brctl addbr $NETIF || true
ifconfig $NETIF ${BASEIP}.1
killall -9 qemu-system-x86
pkill -9 qemu-system-x86

# run dnsmasq
dnsmasq -d \
        --log-queries \
        --bind-dynamic \
        --interface=$NETIF \
        --listen-addr=${BASEIP}.1 \
        --dhcp-range=${BASEIP}.2,${BASEIP}.254,255.255.255.0,12h \
	&> $(pwd)/dnsmasq.log &

function cleanup {
	# kill all children (evil)
	rm $(pwd)/dnsmasq.log
	ifconfig $NETIF down
	brctl delbr $NETIF
	killall -9 qemu-system-x86 dnsmasq
	pkill -9 qemu-system-x86 dnsmasq
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

		# make sure that the server has properly started
		sleep 5

		ip=`cat $(pwd)/dnsmasq.log | \
			grep "dnsmasq-dhcp: DHCPACK(${NETIF})" | \
			tail -n 1 | awk  '{print $3}'`

		# benchmark
		taskset -c ${CPU3},${CPU4} wrk -t 14 -d20s -c 30 \
				http://${ip}/index.html | tee -a $LOG
		#curl http://${BASEIP}.2/index.html --noproxy ${BASEIP}.2 --output -

		# stop server
		killall -9 qemu-system-x86
		pkill -9 qemu-system-x86
	done

	op=`cat $LOG | awk -e '$0 ~ /Requests\/sec: / {print $2}' | \
		sed -r '/^\s*$/d'`
	echo "${op}" >> $RESULTS
done
