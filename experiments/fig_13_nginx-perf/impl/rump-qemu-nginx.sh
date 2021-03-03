#!/bin/bash

# verbose output
set -x

source ../common/set-cpus.sh
source ../common/network.sh
source ../common/nginx.sh
source ../common/qemu.sh

IMAGES=$(pwd)/images/
BASEIP=172.190.0
NETIF=tap10
LOG=rawdata/rump-qemu-nginx.txt
mkdir -p rawdata
touch $LOG

kill_qemu

dnsmasq_pid=$(run_dhcp $NETIF $BASEIP)

function cleanup {
	# kill all children (evil)
	kill_dhcp $dnsmasq_pid
	kill_qemu
	delete_tap $NETIF
	pkill -P $$
	rm ${IMAGES}/rump-qemu.img.disposible
}

trap "cleanup" EXIT

create_tap $NETIF

for j in {1..5}
do
	cp ${IMAGES}/rump-qemu.img ${IMAGES}/rump-qemu.img.disposible

	taskset -c ${CPU1},${CPU2} ${IMAGES}/rump/root/rumprun/app-tools/rumprun \
			kvm -i -M 1024 -g '-daemonize' \
        		-I if,vioif,"-net tap,script=no,ifname=$NETIF" \
        		-W if,inet,dhcp \
			-b ${IMAGES}/rump-qemu.iso,/data \
        		-- ${IMAGES}/rump-qemu.img

	# make sure that the server has properly started
	sleep 15

	ip=`cat $(pwd)/dnsmasq.log | grep "dnsmasq-dhcp: DHCPACK(${NETIF})" | \
		tail -n 1 | awk  '{print $3}'`

	# benchmark
	benchmark_nginx_server $ip $LOG
	#curl http://${ip}/index.html --noproxy ${ip} --output -

	# stop server
	kill_qemu
	rm ${IMAGES}/rump-qemu.img.disposible
done
