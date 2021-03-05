#!/bin/bash

BUILDDIR=../
IMAGES=images/

source ../common/build.sh

mkdir -p $IMAGES

# Generate Lupine VM image

LUPINEDIR=${BUILDDIR}/.lupine

KERNELBUILD="${LUPINEDIR}/Lupine-Linux/kernelbuild/"
LUPINE_KPATH="${KERNELBUILD}/lupine-djw-kml++hello-world/vmlinux"
LUPINENOKML_KPATH="${KERNELBUILD}/lupine-djw-nokml++hello-world/vmlinux"
FIRECRACKER_PATH=.firecracker

build_lupine

# KML
if [ -f "$LUPINE_KPATH" ]; then
	cp $LUPINE_KPATH ${IMAGES}/lupine.img
else
	echo "Lupine (kml) not built, someting went wrong"
	exit 1
fi

# NOKML
if [ -f "$LUPINENOKML_KPATH" ]; then
	cp $LUPINENOKML_KPATH ${IMAGES}/lupine-nokml.img
else
	echo "Lupine (nokml) not built, someting went wrong"
	exit 1
fi

if [ ! -f "$FIRECRACKER_PATH" ]; then
	ln -s ${LUPINEDIR}/Lupine-Linux/firecracker $FIRECRACKER_PATH
fi

rm ${LUPINEDIR}/Lupine-Linux/measure-boot.ext2
pushd ${LUPINEDIR}/Lupine-Linux/
pushd docker
mv measure-boot.Dockerfile Dockerfile
docker build -t measure-boot .
popd
sed -i -e "s/seek=20G/seek=50M/" ./scripts/image2rootfs.sh
./scripts/image2rootfs.sh measure-boot latest ext2
popd

mv ${LUPINEDIR}/Lupine-Linux/measure-boot.ext2 ${IMAGES}/lupine-portwrite.ext2

# Generate Alpine VM image

if [ ! -f "${IMAGES}/alpine.raw" ]; then
	./alpine-image/alpine-make-vm-image -s 300M -f raw \
		--script-chroot $IMAGES/alpine.raw -- ./alpine-image/alpine-configure.sh
fi

if [ ! -f "${IMAGES}/hello-vmlinux.bin" ]; then
	curl -fsSL -o ${IMAGES}/hello-vmlinux.bin https://s3.amazonaws.com/spec.ccfc.min/img/hello/kernel/hello-vmlinux.bin
fi

if [ ! -f "${IMAGES}/mirage-noop.hvt" ]; then
	./mirage-image.sh
fi
