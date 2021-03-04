#!/bin/bash

# verbose output
set -x

source ../common/set-cpus.sh
source ../common/network.sh
source ../common/redis.sh

function hello_hermitux_with_mem {
    IMAGES=$(pwd)/images
    HERMITUX_KERN_LOCATION=/root/hermitux/hermitux-kernel/prefix/x86_64-hermit/extra/tests/hermitux
    UHYVE_LOCATION=/root/hermitux/hermitux-kernel/prefix/bin/proxy
    HELLO_DIR=/root/hermitux/apps/hello-world/
    HELLO_LOCATION=${HELLO_DIR}/prog
    CONTAINER=hermitux-hello-memusage

    docker pull olivierpierre/hermitux
    docker run --privileged --name=$CONTAINER --cpuset-cpus="${CPU1}-${CPU4}" \
		-dt olivierpierre/hermitux
    docker exec $CONTAINER make -C $HELLO_DIR

    {
      sleep 4
      pkill -f -9 hermitux-kernel
      docker container stop $CONTAINER
      docker rm -f $CONTAINER
    } &

    (docker exec $CONTAINER bash -c \
	"HERMIT_ISLE=uhyve HERMIT_TUX=1 HERMIT_MEM=${1}M \
	 ${UHYVE_LOCATION} ${HERMITUX_KERN_LOCATION} ${HELLO_LOCATION}" \
    ) > .out

    wait

    grep -q hello .out
    up=$?
    rm .out

    exit $up
}

function sqlite_hermitux_with_mem {
    IMAGES=$(pwd)/images
    HERMITUX_KERN_LOCATION=/root/hermitux/hermitux-kernel/prefix/x86_64-hermit/extra/tests/hermitux
    UHYVE_LOCATION=/root/hermitux/hermitux-kernel/prefix/bin/proxy
    SQLITE_DIR=/root/hermitux/apps/sqlite/
    SQLITE_LOCATION=${SQLITE_DIR}/prog
    CONTAINER=hermitux-sqlite-memusage

    docker pull olivierpierre/hermitux
    docker run --privileged --name=$CONTAINER --cpuset-cpus="${CPU1}-${CPU4}" \
		-dt olivierpierre/hermitux
    docker exec $CONTAINER bash -c \
	"cd $SQLITE_DIR && sed -i 's/\/Desktop//g' Makefile"
    docker exec $CONTAINER make -C $SQLITE_DIR

    {
      sleep 4
      pkill -f -9 hermitux-kernel
      docker container stop $CONTAINER
      docker rm -f $CONTAINER
    } &

    (docker exec $CONTAINER bash -c \
	"HERMIT_ISLE=uhyve HERMIT_TUX=1 HERMIT_MEM=${1}M \
	 ${UHYVE_LOCATION} ${HERMITUX_KERN_LOCATION} ${SQLITE_LOCATION}" \
    ) > .out

    wait

    grep -q "Retrieval took" .out
    up=$?
    rm .out

    exit $up
}

function redis_hermitux_with_mem {
    IMAGES=$(pwd)/images
    HERMITUX_KERN_LOCATION=/root/hermitux/hermitux-kernel/prefix/x86_64-hermit/extra/tests/hermitux
    UHYVE_LOCATION=/root/hermitux/hermitux-kernel/prefix/bin/proxy
    REDIS_DIR=/root/hermitux/apps/redis-2.0.4/
    REDIS_LOCATION=${REDIS_DIR}/redis-server
    CONTAINER=hermitux-redis-memusage

    docker pull olivierpierre/hermitux
    docker run --privileged --name=$CONTAINER --cpuset-cpus="${CPU1}-${CPU4}" \
		-dt olivierpierre/hermitux
    docker exec $CONTAINER make -C $REDIS_DIR

    {
      sleep 4
      pkill -f -9 hermitux-kernel
      docker container stop $CONTAINER
      docker rm -f $CONTAINER
    } &

    (docker exec $CONTAINER bash -c \
	"HERMIT_ISLE=uhyve HERMIT_TUX=1 HERMIT_MEM=${1}M \
	 ${UHYVE_LOCATION} ${HERMITUX_KERN_LOCATION} ${REDIS_LOCATION}" \
    ) > .out

    wait

    grep -q "0 clients connected" .out
    up=$?
    rm .out

    exit $up
}
