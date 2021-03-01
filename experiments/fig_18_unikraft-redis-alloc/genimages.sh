#!/bin/bash

set -x

IMAGES=$(pwd)/images

rm -rf $IMAGES
mkdir -p $IMAGES

# ========================================================================
# Generate Unikraft images
# ========================================================================

function build_for {
    CONTAINER=uk-tmp-redis
    # kill zombies
    docker container stop $CONTAINER
    docker rm -f $CONTAINER
    sleep 6
    docker pull hlefeuvre/unikraft-eurosys21:latest
    docker run --rm --privileged --name=$CONTAINER \
			-dt hlefeuvre/unikraft-eurosys21
    docker exec -it $CONTAINER bash -c \
	"cd app-${1} && cp configs/${2}.conf .config"
    docker exec -it $CONTAINER bash -c \
	"cd app-${1} && make prepare && make -j"
    docker cp ${CONTAINER}:/root/workspace/apps/app-${1}/build/app-${1}_kvm-x86_64 \
		${IMAGES}/unikraft+${2}.kernel
    docker container stop $CONTAINER
    docker rm -f $CONTAINER
    sleep 6
}

build_for redis tinyalloc
build_for redis mimalloc
build_for redis tlsf
build_for redis buddy
