#!/bin/bash

# verbose output
set -x

source ../common/set-cpus.sh

IMGLOG=/tmp/imglog
FIRECRACKER_PATH=$(pwd)/.firecracker
VMMSOCKET=$(pwd)/.firecracker.socket
BASEIP=172.190.0

function firecracker_cleanup {
  sudo killall -9 firecracker firectl .firecracker
  sudo rm -f $VMMSOCKET
}

function hello_lupine_with_mem {
    IMAGES=$(pwd)/images
    cp ${IMAGES}/hello.ext2 ${IMAGES}/hello.ext2.disp

    {
      sleep 3
      rm ${IMAGES}/hello.ext2.disp
      firecracker_cleanup
    } &

    firectl --firecracker-binary=${FIRECRACKER_PATH} \
	--kernel ${IMAGES}/lupine-hello.kernel --memory $1 \
	--kernel-opts='panic=-1 noapic nomodules pci=off console=ttyS0 init=/hello' \
	--ncpus=1 --root-drive=${IMAGES}/hello.ext2.disp \
        --socket-path=$VMMSOCKET > .out 2> /dev/null

    wait

    grep -q Hello .out
    up=$?
    rm .out

    exit $up
}

function sqlite_lupine_with_mem {
    IMAGES=$(pwd)/images
    cp ${IMAGES}/sqlite.ext2 ${IMAGES}/sqlite.ext2.disp

    {
      sleep 2
      rm ${IMAGES}/sqlite.ext2.disp
      firecracker_cleanup
    } &

    firectl --firecracker-binary=${FIRECRACKER_PATH} \
	--kernel ${IMAGES}/nginx/lupine-fc.kernel --memory $1 \
	--kernel-opts='panic=-1 noapic nomodules pci=off console=ttyS0 init=/guest_start.sh /usr/bin/sqlite3' \
	--ncpus=1 --root-drive=${IMAGES}/sqlite.ext2.disp \
        --socket-path=$VMMSOCKET > .out 2> /dev/null

    wait

    grep -q "sqlite>" .out
    up=$?
    rm .out

    exit $up
}

function redis_lupine_with_mem {
    IMAGES=$(pwd)/images/redis
    NETIF=tap100
    create_tap ${NETIF} ${BASEIP}
    cp ${IMAGES}/redis.ext2 /tmp/redis.ext2.disposible

    {
      sleep 3
      #ping -q -c1 ${BASEIP}.2 > /dev/null
      # this what the Lupine paper did:
      grep -iq "Ready to accept connections" ${IMGLOG}

      if [ $? -eq 0 ]
      then
        firecracker_cleanup
	echo "It's alive!"
        exit 0
      else
        firecracker_cleanup
	echo "Server not alive"
        exit 1
      fi
    } &

    checker_id=$!

    firectl --firecracker-binary=./.firecracker \
            --kernel ${IMAGES}/lupine-fc.kernel \
            --tap-device=tap100/AA:FC:00:00:00:01 \
            --root-drive=/tmp/redis.ext2.disposible \
            --ncpus=1 --memory=$1 \
            --kernel-opts="console=ttyS0 panic=-1 pci=off tsc=reliable ipv6.disable=1 init=/guest_start.sh /trusted/redis-server" > ${IMGLOG} 2> /dev/null &

    worker=$!
    wait $checker_id
    up=$?

    kill -9 $worker
    delete_tap $NETIF
    firecracker_cleanup
    rm /tmp/redis.ext2.disposible
    rm ${IMGLOG}

    exit $up
}

function nginx_lupine_with_mem {
    IMAGES=$(pwd)/images/nginx
    NETIF=tap100
    create_tap ${NETIF} ${BASEIP}
    cp ${IMAGES}/nginx.ext2 /tmp/nginx.ext2.disposible

    {
      sleep 3
      #ping -q -c1 ${BASEIP}.2 > /dev/null
      # this what the Lupine paper did:
      grep -iq "nginx" ${IMGLOG}

      if [ $? -eq 0 ]
      then
        firecracker_cleanup
	echo "It's alive!"
        exit 0
      else
        firecracker_cleanup
	echo "Server not alive"
        exit 1
      fi
    } &

    checker_id=$!

    firectl --firecracker-binary=./.firecracker \
            --kernel ${IMAGES}/lupine-fc.kernel \
            --tap-device=tap100/AA:FC:00:00:00:01 \
            --root-drive=/tmp/nginx.ext2.disposible \
            --ncpus=1 --memory=$1 \
            --kernel-opts="console=ttyS0 panic=-1 pci=off tsc=reliable ipv6.disable=1 init=/guest_start.sh /trusted/nginx" > ${IMGLOG} 2> /dev/null &

    worker=$!

    wait $checker_id
    up=$?

    kill -9 $worker
    delete_tap $NETIF
    firecracker_cleanup
    rm /tmp/nginx.ext2.disposible
    rm ${IMGLOG}

    exit $up
}
