#!/bin/bash

# verbose output
set -x

source ../common/set-cpus.sh
source ../common/network.sh
source ../common/nginx.sh
source ../common/qemu.sh

IMAGES=$(pwd)/images/
BASEIP=172.190.0
NETIF=tux0
LOG=rawdata/osv-qemu-nginx.txt
mkdir -p rawdata
touch $LOG

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
	rm ${IMAGES}/osv-qemu.img.disposible
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

	ip=`cat $(pwd)/dnsmasq.log | grep "dnsmasq-dhcp: DHCPACK(${NETIF})" | \
		tail -n 1 | awk  '{print $3}'`

	# benchmark
	benchmark_nginx_server $ip $LOG
	#curl http://${ip}/index.html --noproxy ${ip} --output -

	# stop server
	kill_qemu
	rm ${IMAGES}/osv-qemu.img.disposible
done

RESULTS=results/osv-qemu.csv
echo "throughput_reqs" > $RESULTS
parse_nginx_results $LOG $RESULTS
