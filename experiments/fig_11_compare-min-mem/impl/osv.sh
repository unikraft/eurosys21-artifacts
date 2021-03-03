#!/bin/bash

# verbose output
set -x

source ../common/set-cpus.sh
source ../common/network.sh
source ../common/redis.sh
source ../common/qemu.sh

IMAGES=$(pwd)/images/

function hello_osv_with_mem {
    IMAGES=$(pwd)/images
    CONTAINER=osv-hello-ramusage
    docker pull hlefeuvre/osv
    docker run --privileged --name=$CONTAINER -dt hlefeuvre/osv

    {
      sleep 4
      docker container stop $CONTAINER
      docker rm -f $CONTAINER
      sleep 3
    } &

    (docker exec $CONTAINER ./scripts/run.py -m $1) > .out 2> /dev/null

    wait

    grep -q Hello .out
    up=$?
    rm .out

    exit $up
}

function sqlite_osv_with_mem {
    IMAGES=$(pwd)/images
    CONTAINER=osv-sqlite-ramusage
    docker pull hlefeuvre/osv
    docker run --privileged --name=$CONTAINER -dt hlefeuvre/osv
    docker exec $CONTAINER ./scripts/build -j4 fs=rofs image=sqlite

    {
      sleep 4
      docker container stop $CONTAINER
      docker rm -f $CONTAINER
      sleep 3
    } &

    (docker exec $CONTAINER ./scripts/run.py -m $1) > .out 2> /dev/null

    wait

    grep -q "sqlite>" .out
    up=$?
    rm .out

    exit $up
}

function redis_osv_with_mem {
    IMAGES=$(pwd)/images/redis
    NETIF=tux0
    create_bridge $NETIF $BASEIP
    dnsmasq_pid=$(run_dhcp $NETIF $BASEIP)

    cp ${IMAGES}/osv-qemu.img ${IMAGES}/osv-qemu.img.disposible

    ${IMAGES}/osv/root/osv/scripts/imgedit.py setargs \
	    ${IMAGES}/osv-qemu.img.disposible \
	    "--rootfs=zfs /redis-server redis.conf"

    {
      sleep 3
      ip=`cat $(pwd)/dnsmasq.log | grep "dnsmasq-dhcp: DHCPACK(${NETIF})" | \
		tail -n 1 | awk  '{print $3}'`

      # benchmark
      ping -q -c 1 $ip
      up=$?

      delete_bridge $NETIF
      kill_dhcp $dnsmasq_pid
      rm ${IMAGES}/osv-qemu.img.disposible
      kill_qemu

      exit $up
    } &

    checker_id=$!

    taskset -c ${CPU1} qemu-guest \
	    -q ${IMAGES}/osv-qemu.img.disposible \
	    -m ${1} -p ${CPU2} \
	    -b ${NETIF} -x

    child_pid=$!

    wait $checker_id
    up=$?

    # stop server
    kill -9 $child_pid

    exit $up
}

function nginx_osv_with_mem {
    IMAGES=$(pwd)/images/nginx
    NETIF=tux0
    create_bridge $NETIF $BASEIP
    dnsmasq_pid=$(run_dhcp $NETIF $BASEIP)

    cp ${IMAGES}/osv-qemu.img ${IMAGES}/osv-qemu.img.disposible

    ${IMAGES}/osv/root/osv/scripts/imgedit.py setargs \
	    ${IMAGES}/osv-qemu.img.disposible \
	    "--rootfs=ramfs /nginx.so -c /nginx/conf/nginx.conf"

    {
      sleep 3
      ip=`cat $(pwd)/dnsmasq.log | grep "dnsmasq-dhcp: DHCPACK(${NETIF})" | \
		tail -n 1 | awk  '{print $3}'`

      # benchmark
      ping -q -c 1 $ip
      up=$?

      delete_bridge $NETIF
      kill_dhcp $dnsmasq_pid
      rm ${IMAGES}/osv-qemu.img.disposible
      kill_qemu

      exit $up
    } &

    checker_id=$!

    taskset -c ${CPU1} qemu-guest \
	    -q ${IMAGES}/osv-qemu.img.disposible \
	    -m $1 -p ${CPU2} \
	    -b ${NETIF} -x

    child_pid=$!

    wait $checker_id
    up=$?

    # stop server
    kill -9 $child_pid

    exit $up
}
