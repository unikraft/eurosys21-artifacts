#!/bin/bash

# verbose output
set -x

source ../common/set-cpus.sh
source ../common/network.sh

IMAGES=$(pwd)/images/
BASEIP=172.190.0
NETIF=tux0
LOG=rawdata/osv-qemu-nginx.txt
mkdir -p rawdata
touch $LOG

echo "creating bridge"
brctl addbr $NETIF || true
ifconfig $NETIF ${BASEIP}.1
killall -9 qemu-system-x86 dnsmasq
pkill -9 qemu-system-x86 dnsmasq

# run dnsmasq
dnsmasq -d \
        --log-queries \
        --bind-dynamic \
        --interface=$NETIF \
        --listen-addr=${BASEIP}.1 \
        --dhcp-range=${BASEIP}.2,${BASEIP}.254,255.255.255.0,12h &> $(pwd)/dnsmasq.log &

function cleanup {
	# kill all children (evil)
	ifconfig $NETIF down
	brctl delbr $NETIF
	rm ${IMAGES}/osv-qemu.img.disposible
	rm dnsmasq.log
	killall -9 qemu-system-x86 dnsmasq
	pkill -9 qemu-system-x86 dnsmasq
	pkill -P $$
}

trap "cleanup" EXIT

for j in {1..5}
do
	cp ${IMAGES}/osv-qemu.img ${IMAGES}/osv-qemu.img.disposible

	# note: --verbose kernel argument for verbose output
	#       and -T nginx argument to dump config :)
	${IMAGES}/osv/root/osv/scripts/imgedit.py setargs \
		${IMAGES}/osv-qemu.img.disposible \
		"--rootfs=ramfs /nginx.so -c /nginx/conf/nginx.conf"

	taskset -c ${CPU1} qemu-guest \
		-q ${IMAGES}/osv-qemu.img.disposible \
                -m 1024 -p ${CPU2} \
		-b ${NETIF} -x

	# make sure that the server has properly started
	sleep 6

	ip=`cat $(pwd)/dnsmasq.log | grep "dnsmasq-dhcp: DHCPACK(${NETIF})" | tail -n 1 | awk  '{print $3}'`

	# benchmark
	taskset -c ${CPU3},${CPU4} $WRK -t 14 -d1m -c 30 \
			http://${ip}/index.html >> $LOG
	#curl http://${ip}/index.html --noproxy ${ip} --output -

	# stop server
	killall -9 qemu-system-x86
	pkill -9 qemu-system-x86
	rm ${IMAGES}/osv-qemu.img.disposible
done
