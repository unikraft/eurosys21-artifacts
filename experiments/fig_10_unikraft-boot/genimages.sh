#!/bin/bash

set -x

IMAGES=$(pwd)/images

source ../common/build.sh

rm -rf $IMAGES
mkdir -p $IMAGES

# ========================================================================
# Firecracker binary
# ========================================================================

CONTAINER=instvmm-tmp
# kill zombies
docker container stop $CONTAINER
docker rm -f $CONTAINER
sleep 3
docker pull hlefeuvre/instrumented-vmms:latest
docker run --rm --privileged --name=$CONTAINER \
    		-dt hlefeuvre/instrumented-vmms
docker cp ${CONTAINER}:/root/firecracker ${IMAGES}/firecracker
docker cp ${CONTAINER}:/root/qemu ${IMAGES}/qemu
docker container stop $CONTAINER
docker rm -f $CONTAINER

# ========================================================================
# Generate KVM images
# ========================================================================

unikraft_eurosys21_build helloworld stat $IMAGES
mv $IMAGES/unikraft+stat.kernel $IMAGES/unikraft+qemu.kernel

unikraft_eurosys21_build helloworld qemu1nic $IMAGES

# ========================================================================
# Generate Solo5 images
# ========================================================================

unikraft_eurosys21_build_wvmm helloworld solo5 $IMAGES solo5

# ========================================================================
# Generate Firecracker images
# ========================================================================

unikraft_eurosys21_build_wvmm helloworld-boottime-fc firecracker $IMAGES kvmfc
