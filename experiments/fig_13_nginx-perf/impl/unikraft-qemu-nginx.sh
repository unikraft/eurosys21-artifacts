#!/bin/bash

# verbose output
set -x

source ../common/set-cpus.sh
source ../common/nginx.sh

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

LOG=rawdata/unikraft-qemu-nginx.txt
touch $LOG

for j in {1..5}
do
	taskset -c ${CPU1} qemu-guest \
		-i nginx.cpio \
		-k ${IMAGES}/unikraft+mimalloc.kernel \
		-a "" -m 1024 -p ${CPU2} \
		-b ${NETIF} -x

	# make sure that the server has properly started
	sleep 5

	ip=`cat $(pwd)/dnsmasq.log | \
		grep "dnsmasq-dhcp: DHCPACK(${NETIF})" | \
		tail -n 1 | awk  '{print $3}'`

	# benchmark
	benchmark_nginx_server ${ip} $LOG
	#curl http://${BASEIP}.2/index.html --noproxy ${BASEIP}.2 --output -

	# stop server
	killall -9 qemu-system-x86
	pkill -9 qemu-system-x86
done
