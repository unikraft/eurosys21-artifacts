#!/bin/bash

# verbose output
set -x

source ../common/set-cpus.sh
source ../common/network.sh
source ../common/redis.sh
source ../common/qemu.sh

function hello_osv_with_mem {
    CONTAINER=osv-hello-ramusage
    docker pull hlefeuvre/osv
    docker run --privileged --name=$CONTAINER -dt hlefeuvre/osv

    {
      sleep 4
      docker container stop $CONTAINER
      docker rm -f $CONTAINER
      sleep 3
    } &

    (docker exec $CONTAINER ./scripts/firecracker.py -m $1) > .out 2> /dev/null

    wait

    grep -q Hello .out
    up=$?
    rm .out

    exit $up
}

function sqlite_osv_with_mem {
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

    (docker exec $CONTAINER ./scripts/firecracker.py -m $1) > .out 2> /dev/null

    wait

    grep -q "sqlite>" .out
    up=$?
    rm .out

    exit $up
}

function redis_osv_with_mem {
    CONTAINER=osv-redis-ramusage
    docker pull hlefeuvre/osv
    docker run --privileged --name=$CONTAINER -dt hlefeuvre/osv
    docker exec $CONTAINER ./scripts/build -j4 fs=ramfs image=redis-memonly

    {
      sleep 4
      docker container stop $CONTAINER
      docker rm -f $CONTAINER
      sleep 3
    } &

    (docker exec $CONTAINER ./scripts/firecracker.py -m $1) > .out 2> /dev/null

    wait

    grep -q "ready to accept connections" .out
    up=$?
    rm .out

    exit $up
}

function nginx_osv_with_mem {
    CONTAINER=osv-nginx-ramusage
    docker pull hlefeuvre/osv
    docker run --privileged --name=$CONTAINER -dt hlefeuvre/osv
    docker exec $CONTAINER sed -i "s/#error_log stderr debug;/error_log stderr debug;/g" \
	    apps/nginx/patches/nginx.conf
    docker exec $CONTAINER ./scripts/build -j4 fs=ramfs image=nginx

    {
      sleep 4
      docker container stop $CONTAINER
      docker rm -f $CONTAINER
      sleep 3
    } &

    (docker exec $CONTAINER ./scripts/firecracker.py -m $1) > .out 2> /dev/null

    wait

    grep -q "start worker processes" .out
    up=$?
    rm .out

    exit $up
}
