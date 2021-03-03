#!/bin/bash

# verbose output
set -x

source ../common/set-cpus.sh
source ../common/network.sh
source ../common/redis.sh
source ../common/qemu.sh

IMAGES=$(pwd)/images

function hello_rump_with_mem {
    IMAGES=$(pwd)/images
    CONTAINER=rump-hello-ram-tmp

    # remove stale containers
    docker container stop $CONTAINER
    docker rm -f $CONTAINER
    sleep 3

    docker pull hlefeuvre/rump-solo5
    docker run --privileged --name=$CONTAINER -dt hlefeuvre/rump-solo5
    docker exec $CONTAINER bash -c \
		". /root/rumprun/obj/config && ./runtests.sh hvt" &> /dev/null
    docker exec $CONTAINER dd if=/dev/zero of=hello.disk bs=512 count=1

    {
      sleep 8
      docker container stop $CONTAINER
      docker rm -f $CONTAINER
      sleep 3
    } &

    (docker exec $CONTAINER bash -c ". /root/rumprun/obj/config && \
				/root/rumprun/solo5/tenders/hvt/solo5-hvt \
				--net:tap=tap100 --mem=$1 \
				--block:rootfs=hello.disk \
				/root/rumprun/tests/hello/hello.bin" \
    ) > .out 2> /dev/null
    wait

    grep -q Hello .out
    up=$?
    rm .out

    exit $up
}

function sqlite_rump_with_mem {
    IMAGES=$(pwd)/images
    CONTAINER=rump-sqlite-ram-tmp

    # remove stale containers
    docker container stop $CONTAINER
    docker rm -f $CONTAINER
    sleep 3

    docker pull hlefeuvre/rump-solo5
    docker run --privileged --name=$CONTAINER -dt hlefeuvre/rump-solo5
    docker exec $CONTAINER bash -c \
		". /root/rumprun/./obj/config && ./runtests.sh hvt" &> /dev/null
    docker exec $CONTAINER bash -c "cd /root/rumprun-packages/pkgs/bin/ && \
			dd if=/dev/zero of=hello.disk bs=512 count=1"

    {
      sleep 4
      docker container stop $CONTAINER
      docker rm -f $CONTAINER
      sleep 3
    } &

    (docker exec $CONTAINER bash -c "cd /root/rumprun-packages/pkgs/bin/ && \
			. /root/rumprun/obj/config && \
			/root/rumprun/solo5/tenders/hvt/solo5-hvt \
			--net:tap=tap100 --mem=${1}M \
			--block:rootfs=hello.disk sqlite3.img" \
    ) > .out 2> /dev/null
    wait

    grep -q "sqlite>" .out
    up=$?
    rm .out

    exit $up
}

function redis_rump_with_mem {
    IMAGES=$(pwd)/images/redis
    NETIF=tap10

    kill_qemu

    create_tap $NETIF $BASEIP
    dnsmasq_pid=$(run_dhcp $NETIF $BASEIP)

    cp ${IMAGES}/rump-qemu.img ${IMAGES}/rump-qemu.img.disposible

    {
      sleep 20
      ip=`cat $(pwd)/dnsmasq.log | grep "dnsmasq-dhcp: DHCPACK(${NETIF})" | \
		tail -n 1 | awk  '{print $3}'`

      # benchmark
      ping -q -c 1 $ip
      up=$?

      kill_dhcp $dnsmasq_pid
      kill_qemu
      delete_tap $NETIF

      rm ${IMAGES}/rump-qemu.img.disposible

      exit $up
    } &

    checker_id=$!

    taskset -c ${CPU1},${CPU2} ${IMAGES}/rump/root/rumprun/app-tools/rumprun \
		    kvm -i -M $1 -g '-daemonize' \
		    -I if,vioif,"-net tap,script=no,ifname=$NETIF" \
		    -W if,inet,dhcp \
		    -b ${IMAGES}/rump-qemu.iso,/data \
		    -- ${IMAGES}/rump-qemu.img /data/conf/redis.conf

    wait $checker_id
    up=$?

    exit $up
}

function nginx_rump_with_mem {
    IMAGES=$(pwd)/images/nginx
    NETIF=tap10

    kill_qemu

    create_tap $NETIF $BASEIP
    dnsmasq_pid=$(run_dhcp $NETIF $BASEIP)

    cp ${IMAGES}/rump-qemu.img ${IMAGES}/rump-qemu.img.disposible

    {
      sleep 20
      ip=`cat $(pwd)/dnsmasq.log | grep "dnsmasq-dhcp: DHCPACK(${NETIF})" | \
		tail -n 1 | awk  '{print $3}'`

      # benchmark
      ping -q -c 1 $ip
      up=$?

      kill_dhcp $dnsmasq_pid
      kill_qemu

      delete_tap $NETIF

      rm ${IMAGES}/rump-qemu.img.disposible

      exit $up
    } &

    checker_id=$!

    taskset -c ${CPU1},${CPU2} ${IMAGES}/rump/root/rumprun/app-tools/rumprun \
		    kvm -i -M $1 -g '-daemonize' \
		    -I if,vioif,"-net tap,script=no,ifname=$NETIF" \
		    -W if,inet,dhcp \
		    -b ${IMAGES}/rump-qemu.iso,/data \
		    -- ${IMAGES}/rump-qemu.img

    wait $checker_id
    up=$?

    exit $up
}
