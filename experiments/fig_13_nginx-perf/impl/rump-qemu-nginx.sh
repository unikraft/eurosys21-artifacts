#!/bin/bash

# verbose output
set -x

source ../common/set-cpus.sh
source ../common/network.sh

IMAGES=$(pwd)/images/
BASEIP=172.190.0
NETIF=tap10
LOG=rawdata/rump-qemu-nginx.txt
mkdir -p rawdata
touch $LOG

killall -9 qemu-system-x86 dnsmasq
pkill -9 qemu-system-x86 dnsmasq

create_tap() {
    if ! ip link show tap$1 &> /dev/null; then
        sudo ip tuntap add mode tap tap$1
        sudo ip addr add ${BASEIP}.1/24 dev tap$1
        sudo ip link set tap$1 up
        echo 1 | sudo tee /proc/sys/net/ipv4/ip_forward >/dev/null
        sudo iptables -t nat -A POSTROUTING -o bond1 -j MASQUERADE
        sudo iptables -I FORWARD 1 -i tap$1 -j ACCEPT
        sudo iptables -I FORWARD 1 -o tap$1 -m state --state RELATED,ESTABLISHED -j ACCEPT
    fi
}

delete_tap() {
    sudo ip link del tap$1
}

# run dnsmasq
dnsmasq -d \
        --log-queries \
        --bind-dynamic \
        --interface=$NETIF \
        --listen-addr=${BASEIP}.1 \
        --dhcp-range=${BASEIP}.2,${BASEIP}.254,255.255.255.0,12h &> $(pwd)/dnsmasq.log &

function cleanup {
	# kill all children (evil)
	delete_tap 10
	rm ${IMAGES}/rump-qemu.img.disposible
	rm dnsmasq.log
	killall -9 qemu-system-x86 dnsmasq
	pkill -9 qemu-system-x86 dnsmasq
	pkill -P $$
}

trap "cleanup" EXIT

create_tap 10

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

	ip=`cat $(pwd)/dnsmasq.log | grep "dnsmasq-dhcp: DHCPACK(${NETIF})" | tail -n 1 | awk  '{print $3}'`

	# benchmark
	taskset -c ${CPU3},${CPU4} $WRK -t 14 -d1m -c 30 \
			http://${ip}/index.html >> $LOG
	#curl http://${ip}/index.html --noproxy ${ip} --output -

	# stop server
	killall -9 qemu-system-x86
	pkill -9 qemu-system-x86
	rm ${IMAGES}/rump-qemu.img.disposible
done
