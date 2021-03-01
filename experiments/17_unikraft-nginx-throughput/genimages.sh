#!/bin/bash

set -x

IMAGES=$(pwd)/images

rm -rf $IMAGES
mkdir -p $IMAGES

# ========================================================================
# Generate Unikraft images
# ========================================================================

function build_for {
    CONTAINER=uk-tmp-nginx
    # kill zombies
    docker container stop $CONTAINER
    docker rm -f $CONTAINER
    sleep 6
    #docker pull hlefeuvre/unikraft-nwbench:latest
    docker run --rm --privileged --name=$CONTAINER \
			-dt unikraft-nwbench:latest
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

build_for nginx tinyalloc
build_for nginx mimalloc
build_for nginx tlsf
build_for nginx buddy