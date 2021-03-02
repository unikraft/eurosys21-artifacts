#!/bin/bash

set -x

BUILDDIR=../
IMAGES=$(pwd)/images/

source ../common/build.sh

mkdir -p $IMAGES
mkdir -p data

# ========================================================================
# Generate nginx images (using nginx/ genimages script)
# ========================================================================

${BUILDDIR}/fig_13_nginx-perf/genimages.sh
cp -r ${BUILDDIR}/fig_13_nginx-perf/data/* data/
# in case redis/genimages.sh cleans up $IMAGES
IMAGES=$(pwd)/images/
mv $IMAGES nginx

# ========================================================================
# Generate redis images (using redis/ genimages script)
# ========================================================================

${BUILDDIR}/fig_12_redis-perf/genimages.sh
cp -r ${BUILDDIR}/fig_12_redis-perf/data/* data/
# see previous comment
IMAGES=$(pwd)/images/
mv $IMAGES redis

# ========================================================================
# Finalize
# ========================================================================

mkdir -p $IMAGES
mv redis $IMAGES
mv nginx $IMAGES

# ========================================================================
# Generate Lupine VM images + hello/sqlite ext2 rootfs
# ========================================================================

build_lupine

LUPINEDIR=${BUILDDIR}/.lupine

LUPINE_KPATH="${LUPINEDIR}/Lupine-Linux/kernelbuild/lupine-djw-kml-tiny++hello-world/vmlinux"
FIRECRACKER_PATH=.firecracker
if [ ! -f "$LUPINE_KPATH" ]; then
	echo "Lupine not built, something went wrong..."
	exit 1
fi

pushd ${LUPINEDIR}/Lupine-Linux/
sed -i -e "s/seek=20G/seek=30M/" ./scripts/image2rootfs.sh

# build image
./scripts/image2rootfs.sh hello-world latest ext2
docker pull hlefeuvre/alpine-sqlite
docker tag hlefeuvre/alpine-sqlite alpine-sqlite
./scripts/image2rootfs.sh alpine-sqlite latest ext2

# idempotence
git checkout ./scripts/image2rootfs.sh
popd

mv ${LUPINEDIR}/Lupine-Linux/hello-world.ext2 ${IMAGES}/hello.ext2
mv ${LUPINEDIR}/Lupine-Linux/alpine-sqlite.ext2 ${IMAGES}/sqlite.ext2
cp $LUPINE_KPATH images/lupine-hello.kernel

if [ ! -f "$FIRECRACKER_PATH" ]; then
	ln -s ${LUPINEDIR}/Lupine-Linux/firecracker $FIRECRACKER_PATH
fi
