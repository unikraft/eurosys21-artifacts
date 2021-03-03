#!/bin/bash

set -x

IMAGES=$(pwd)/images

rm -rf $IMAGES
mkdir -p $IMAGES

# ========================================================================
# Generate Unikraft images
# ========================================================================

function build_for {
    CONTAINER=uk-tmp
    # kill zombies
    docker container stop $CONTAINER
    docker rm -f $CONTAINER
    docker pull hlefeuvre/unikraft-eurosys21:latest
    # give this one 4 CPUs to build faster
    docker run --rm --privileged --name=$CONTAINER \
	    		--cpuset-cpus="${CPU1}-${CPU4}" \
			-dt hlefeuvre/unikraft-eurosys21
    docker exec -it $CONTAINER bash -c \
	"cd app-${1} && cp configs/${3}.conf .config"
    docker exec -it $CONTAINER sed -i -e "s/60000/${2}/" app-${1}/main.c
    docker exec -it $CONTAINER bash -c \
	"cd app-${1} && make prepare && make -j"
    docker cp ${CONTAINER}:/root/workspace/apps/app-${1}/build/app-${1}_kvm-x86_64 \
		${IMAGES}/${2}/unikraft+${3}.kernel
    docker cp ${CONTAINER}:/root/workspace/apps/app-${1}/${1}.cpio \
		${1}.cpio
    docker container stop $CONTAINER
    docker rm -f $CONTAINER
    sleep 3
}

mkdir -p $IMAGES/10
build_for sqlitebenchmark 10 tinyalloc
build_for sqlitebenchmark 10 mimalloc
build_for sqlitebenchmark 10 tlsf
build_for sqlitebenchmark 10 buddy

mkdir -p $IMAGES/100
build_for sqlitebenchmark 100 tinyalloc
build_for sqlitebenchmark 100 mimalloc
build_for sqlitebenchmark 100 tlsf
build_for sqlitebenchmark 100 buddy

mkdir -p $IMAGES/1000
build_for sqlitebenchmark 1000 tinyalloc
build_for sqlitebenchmark 1000 mimalloc
build_for sqlitebenchmark 1000 tlsf
build_for sqlitebenchmark 1000 buddy

mkdir -p $IMAGES/10000
build_for sqlitebenchmark 10000 tinyalloc
build_for sqlitebenchmark 10000 mimalloc
build_for sqlitebenchmark 10000 tlsf
build_for sqlitebenchmark 10000 buddy

mkdir -p $IMAGES/60000
build_for sqlitebenchmark 60000 tinyalloc
build_for sqlitebenchmark 60000 mimalloc
build_for sqlitebenchmark 60000 tlsf
build_for sqlitebenchmark 60000 buddy

mkdir -p $IMAGES/100000
build_for sqlitebenchmark 100000 tinyalloc
build_for sqlitebenchmark 100000 mimalloc
build_for sqlitebenchmark 100000 tlsf
build_for sqlitebenchmark 100000 buddy
