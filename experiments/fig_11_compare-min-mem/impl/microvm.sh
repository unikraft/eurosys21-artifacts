#!/bin/bash

# verbose output
set -x

source ../common/set-cpus.sh
source ../common/network.sh
source ../common/redis.sh
source ../common/qemu.sh

IMGLOG=/tmp/imglog

function hello_microvm_with_mem {
    IMAGES=$(pwd)/images
    cp ${IMAGES}/hello.ext2 ${IMAGES}/hello.ext2.disp

    {
      sleep 3
      rm ${IMAGES}/hello.ext2.disp
      firecracker_cleanup
    } &

    firectl --firecracker-binary=${FIRECRACKER_PATH} \
	--kernel ${IMAGES}/nginx/generic-fc.kernel --memory $1 \
	--kernel-opts='panic=-1 noapic nomodules pci=off console=ttyS0 init=/hello' \
	--ncpus=1 --root-drive=${IMAGES}/hello.ext2.disp \
        --socket-path=$VMMSOCKET > .out 2> /dev/null

    wait

    grep -q Hello .out
    up=$?
    rm .out

    exit $up
}

function sqlite_microvm_with_mem {
    IMAGES=$(pwd)/images

    cp ${IMAGES}/sqlite.ext2 ${IMAGES}/sqlite.ext2.disp

    {
      sleep 4
      rm ${IMAGES}/sqlite.ext2.disp
      firecracker_cleanup
    } &

    firectl --firecracker-binary=${FIRECRACKER_PATH} \
	--kernel ${IMAGES}/nginx/generic-fc.kernel --memory $1 \
	--kernel-opts='panic=-1 noapic nomodules pci=off console=ttyS0 init=/guest_start.sh /usr/bin/sqlite3' \
	--ncpus=1 --root-drive=${IMAGES}/sqlite.ext2.disp \
        --socket-path=$VMMSOCKET > .out 2> /dev/null

    wait

    grep -q "sqlite>" .out
    up=$?
    rm .out

    exit $up
}

function redis_microvm_with_mem {
    IMAGES=$(pwd)/images/redis
    NETIF=tux0

    create_bridge $NETIF $BASEIP
    kill_qemu

    cp ${IMAGES}/redis.ext2 ${IMAGES}/redis.ext2.disposible

    {
      sleep 3

      # benchmark
      ping -q -c 1 ${BASEIP}.2
      up=$?

      delete_bridge $NETIF
      rm ${IMAGES}/redis.ext2.disposible
      kill_qemu

      exit $up
    } &

    checker_id=$!

    taskset -c ${CPU1} qemu-guest \
	    -k ${IMAGES}/generic-qemu.kernel \
	    -d ${IMAGES}/redis.ext2.disposible \
	    -a "root=/dev/vda rw console=ttyS0 init=/guest_start.sh redis-server" \
	    -m $1 -p ${CPU2}\
	    -b ${NETIF} -x

    child_pid=$!

    wait $checker_id
    up=$?

    # stop server
    kill -9 $child_pid

    exit $up
}

function nginx_microvm_with_mem {
    IMAGES=$(pwd)/images/nginx
    NETIF=tux0

    firecracker_cleanup
    create_tap ${NETIF} ${BASEIP}

    cp ${IMAGES}/nginx.ext2 ${IMAGES}/nginx.ext2.disposible

    {
      sleep 3

      # benchmark
      #ping -q -c 1 ${BASEIP}.2
      grep -iq "nginx" ${IMGLOG}
      up=$?

      delete_tap $NETIF
      rm ${IMAGES}/nginx.ext2.disposible
      firecracker_cleanup

      exit $up
    } &

    checker_id=$!

    firectl --firecracker-binary=./.firecracker \
            --kernel ${IMAGES}/generic-fc.kernel \
            --tap-device=tap100/AA:FC:00:00:00:01 \
            --root-drive=/${IMAGES}/nginx.ext2.disposible \
            --ncpus=1 --memory=$1 \
            --kernel-opts="console=ttyS0 panic=-1 pci=off tsc=reliable ipv6.disable=1 init=/guest_start.sh /trusted/redis-server" > ${IMGLOG} 2> /dev/null &

    child_pid=$!

    wait $checker_id
    up=$?

    # stop server
    kill -9 $child_pid

    exit $up
}
