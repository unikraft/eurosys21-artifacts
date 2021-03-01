#!/bin/bash

set -x

IMAGES=$(pwd)/images

rm -rf $IMAGES
mkdir -p $IMAGES

# ========================================================================
# Generate Unikraft images
# ========================================================================

function build_for {
    CONTAINER=uk-tmp-hello-pt
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
    # important for tracepoints -> dbg binaries
    docker cp ${CONTAINER}:/root/workspace/apps/app-${1}/build/app-${1}_kvm-x86_64.dbg \
		${IMAGES}/unikraft+${2}.kernel
    docker container stop $CONTAINER
    docker rm -f $CONTAINER
    sleep 3
}

build_for helloworld-boottime-pt dyn
build_for helloworld stat

# 32MB measurement needs a hack
# this is temporary (the patch has not been merged yet)
CONTAINER=uk-tmp-hello-pt
# kill zombies
docker container stop $CONTAINER
docker rm -f $CONTAINER
sleep 6
docker pull hlefeuvre/unikraft-eurosys21:latest
docker run --rm --privileged --name=$CONTAINER \
		        -v $(pwd)/data/:/data \
			-dt hlefeuvre/unikraft-eurosys21
docker exec -it $CONTAINER bash -c \
    "cd app-helloworld-boottime-pt && cp configs/dyn.conf .config"
docker exec -it $CONTAINER bash -c \
    "cd ../unikraft-ptsupport && git apply /data/0001-Patch-for-32MB-dynamic.patch"
docker exec -it $CONTAINER bash -c \
    "cd app-helloworld-boottime-pt && make prepare && make -j"
# important for tracepoints -> dbg binaries
docker cp ${CONTAINER}:/root/workspace/apps/app-helloworld-boottime-pt/build/app-helloworld-boottime-pt_kvm-x86_64.dbg \
    	${IMAGES}/unikraft+dyn32.kernel
docker container stop $CONTAINER
docker rm -f $CONTAINER
sleep 3
