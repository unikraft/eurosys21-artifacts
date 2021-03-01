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

# create firecracker disk image (not used, but seems to be required)
qemu-img create -f raw /tmp/fcdisk.img 1K

# create firecracker configuration
cp data/firecracker.config.in data/firecracker.config
sed -i "s|{{KERNELIMAGE}}|${IMAGES}/unikraft+firecracker.kernel|g" \
	data/firecracker.config
sed -i "s|{{LOGS}}|$(pwd)/logs.fifo|g" data/firecracker.config
sed -i "s|{{METRICS}}|$(pwd)/metrics.fifo|g" data/firecracker.config

# ========================================================================
# Generate KVM images
# ========================================================================

unikraft_eurosys21_build helloworld stat $IMAGES
mv $IMAGES/unikraft+stat.kernel $IMAGES/unikraft+qemu.kernel

# ========================================================================
# Generate Solo5 images
# ========================================================================

unikraft_eurosys21_build_wvmm helloworld solo5 $IMAGES solo5

# ========================================================================
# Generate Firecracker images
# ========================================================================

unikraft_eurosys21_build_wvmm helloworld-boottime-fc firecracker $IMAGES kvmfc
