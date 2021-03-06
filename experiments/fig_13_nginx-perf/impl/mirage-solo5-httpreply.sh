#!/bin/bash

# verbose output
set -x

source ../common/set-cpus.sh
source ../common/network.sh
source ../common/nginx.sh

WORKDIR="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
IMAGESDIR=$WORKDIR/../images
TOOLSDIR=$WORKDIR/../../../tools

BASEIP=172.190.0
LOG=$WORKDIR/../rawdata/mirage-solo5-httpreply.txt
mkdir -p $WORKDIR/../rawdata
touch $LOG

modprobe tun

create_tap() {
  if ! ip link show tap$1 &> /dev/null; then
    ip tuntap add mode tap tap$1
    ip addr add ${BASEIP}.1/24 dev tap$1
    ip link set tap$1 up
    echo 1 | tee /proc/sys/net/ipv4/ip_forward >/dev/null
    iptables -t nat -A POSTROUTING -o bond1 -j MASQUERADE
    iptables -I FORWARD 1 -i tap$1 -j ACCEPT
    iptables -I FORWARD 1 -o tap$1 -m state --state RELATED,ESTABLISHED -j ACCEPT
  fi
}

delete_tap() {
  ip link del tap$1
}


function cleanup {
	# kill all children (evil)
	killall -9 solo5-hvt
	pkill -P $$
	delete_tap 100
}

trap "cleanup" EXIT

create_tap 100

for j in {1..5}; do
  taskset -c ${CPU1},${CPU2} $TOOLSDIR/solo5-hvt \
    --net:service=tap100 -- \
    $IMAGESDIR/mirage-solo5-httpreply.hvt \
      --ipv4=${BASEIP}.2/24 \
      --ipv4-gateway=${BASEIP}.1 &

  # make sure that the server has properly started
	sleep 3

  # benchmark
  benchmark_nginx_server ${BASEIP}.2:8080 $LOG

  killall -9 solo5-hvt

  sleep 3
done

mkdir -p $WORKDIR/../results/
RESULTS=$WORKDIR/../results/mirage-solo5.csv
echo "throughput_reqs" > $RESULTS
parse_nginx_results $LOG $RESULTS
