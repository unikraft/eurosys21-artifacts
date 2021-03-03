#!/bin/bash

# verbose output
set -x

source ../common/set-cpus.sh
source ../common/network.sh
source ../common/nginx.sh

IMAGES=images/
BASEIP=172.190.0
LOG=rawdata/lupine-fc-nginx.txt
mkdir -p rawdata
touch $LOG

modprobe tun

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

function cleanup {
	# kill all children (evil)
	killall -9 firecracker firectl cp .firecracker
	rm /tmp/nginx.ext2.disposible
	pkill -P $$
	delete_tap 100
}

trap "cleanup" EXIT

create_tap 100

for j in {1..5}
do
	cp ${IMAGES}/nginx.ext2 /tmp/nginx.ext2.disposible

	taskset -c ${CPU1},${CPU2} firectl --firecracker-binary=./.firecracker \
                --kernel ${IMAGES}/lupine-fc.kernel \
                --tap-device=tap100/AA:FC:00:00:00:01 \
                --root-drive=/tmp/nginx.ext2.disposible \
                --ncpus=1 --memory=1024 \
                --kernel-opts="console=ttyS0 panic=-1 pci=off tsc=reliable ipv6.disable=1 init=/guest_start.sh /trusted/nginx" &

	# make sure that the server has properly started
	sleep 3

	# benchmark
	benchmark_nginx_server ${BASEIP}.2 $LOG
	#curl http://${BASEIP}.2/index.html --noproxy ${BASEIP}.2 --output -	

	# stop server
	killall -9 firecracker firectl .firecracker
	rm /tmp/nginx.ext2.disposible
done

RESULTS=results/lupine-fc.csv
echo "throughput_reqs" > $RESULTS
parse_nginx_results $LOG $RESULTS
