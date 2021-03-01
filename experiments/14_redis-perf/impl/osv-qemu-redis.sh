#!/bin/bash

# verbose output
set -x

NETIF=tux0
LOG=rawdata/osv-qemu-redis.txt
mkdir -p rawdata
touch $LOG

IMAGES=$(pwd)/images

source ../common/set-cpus.sh
source ../common/network.sh
source ../common/redis.sh

create_bridge $NETIF $BASEIP
killall -9 qemu-system-x86
pkill -9 qemu-system-x86

# run dnsmasq
run_dhcp $NETIF $BASEIP

function cleanup {
	# kill all children (evil)
	delete_bridge $NETIF
	kill_dhcp
	rm ${IMAGES}/osv-qemu.img.disposible
	killall -9 qemu-system-x86
	pkill -9 qemu-system-x86
	pkill -P $$
}

trap "cleanup" EXIT

for j in {1..5}
do
	cp ${IMAGES}/osv-qemu.img ${IMAGES}/osv-qemu.img.disposible

	# note: --verbose kernel argument for verbose output
	#       and -T redis argument to dump config :)
	${IMAGES}/osv/root/osv/scripts/imgedit.py setargs \
		${IMAGES}/osv-qemu.img.disposible \
		"--rootfs=zfs /redis-server redis.conf"

	taskset -c ${CPU1} qemu-guest \
		-q ${IMAGES}/osv-qemu.img.disposible \
                -m 1024 -p ${CPU2} \
		-b ${NETIF}

	# make sure that the server has properly started
	sleep 6

	ip=`cat $(pwd)/dnsmasq.log | grep "dnsmasq-dhcp: DHCPACK(${NETIF})" | tail -n 1 | awk  '{print $3}'`

	# benchmark
	benchmark_server ${ip} 6379

	# stop server
	killall -9 qemu-system-x86
	pkill -9 qemu-system-x86
	rm ${IMAGES}/osv-qemu.img.disposible
done
