#!/bin/bash

set -x

IMAGES=$(pwd)/images

source ../common/build.sh

BUILDDIR=..
LUPINEDIR=${BUILDDIR}/.lupine
FIRECRACKER_PATH=.firecracker
SOLO5_PATH=.solo5

rm -rf $SOLO5_PATH $FIRECRACKER_PATH $IMAGES
mkdir -p $IMAGES

# ========================================================================
# Firecracker binary
# ========================================================================

if [ ! -f "$FIRECRACKER_PATH" ]; then
	ln -s ${LUPINEDIR}/Lupine-Linux/firecracker $FIRECRACKER_PATH
fi

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
