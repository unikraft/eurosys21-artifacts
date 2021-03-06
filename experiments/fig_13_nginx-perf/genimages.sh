#!/bin/bash

set -x

BUILDDIR=..
IMAGES=$(pwd)/images
BASEIP=172.190.0
GUESTSTART=$(pwd)/data/guest_start.sh

source ../common/build.sh
source ../common/set-cpus.sh

rm -rf $IMAGES
mkdir -p $IMAGES

# ========================================================================
# Generate Unikraft VM images
# ========================================================================

unikraft_eurosys21_build nginx mimalloc $IMAGES

# ========================================================================
# Generate Lupine VM images + nginx ext2 rootfs
# ========================================================================

LUPINEDIR=${BUILDDIR}/.lupine

build_lupine

KERNELS="${LUPINEDIR}/Lupine-Linux/kernelbuild/"
# uncompressed version for firecracker
LUPINE_FC_KPATH="${KERNELS}/lupine-djw-kml++nginx/vmlinux"
GENERIC_FC_KPATH="${KERNELS}/microvm++nginx/vmlinux"
# compressed one for QEMU
LUPINE_KVM_KPATH="${KERNELS}/lupine-djw-kml-qemu++nginx/vmlinuz-4.0.0-kml"
GENERIC_KVM_KPATH="${KERNELS}/microvm++nginx/vmlinuz-4.0.0"

FIRECRACKER_PATH=.firecracker
if [ -f "$LUPINE_FC_KPATH" ]; then
	cp $LUPINE_FC_KPATH ${IMAGES}/lupine-fc.kernel
	cp $GENERIC_FC_KPATH ${IMAGES}/generic-fc.kernel
	cp $LUPINE_KVM_KPATH ${IMAGES}/lupine-qemu.kernel
	cp $GENERIC_KVM_KPATH ${IMAGES}/generic-qemu.kernel
else
	echo "Lupine not built, something went wrong!"
	exit 1
fi

if [ ! -f "$FIRECRACKER_PATH" ]; then
	ln -s ${LUPINEDIR}/Lupine-Linux/firecracker $FIRECRACKER_PATH
fi

pushd ${LUPINEDIR}/Lupine-Linux/
# various patches...
cp ${GUESTSTART} ./scripts/guest_start.sh
sed -i -e "s/192.168.100/${BASEIP}/" ./scripts/guest_net.sh
sed -i -e "s/seek=20G/seek=30M/" ./scripts/image2rootfs.sh

# build image
./scripts/image2rootfs.sh nginx 1.15.6-alpine ext2

# idempotence
git checkout ./scripts/image2rootfs.sh
git checkout ./scripts/guest_net.sh
git checkout ./scripts/guest_start.sh
popd

mv ${LUPINEDIR}/Lupine-Linux/nginx.ext2 ${IMAGES}/nginx.ext2

modprobe loop
mkdir -p /mnt/nginx-tmp
mount -o loop ${IMAGES}/nginx.ext2 /mnt/nginx-tmp
cp ./data/nginx.conf /mnt/nginx-tmp/etc/nginx/nginx.conf
umount /mnt/nginx-tmp
rm -rf /mnt/nginx-tmp

# ========================================================================
# Generate OSv VM image
# ========================================================================

CONTAINER=osv-tmp
docker pull hlefeuvre/osv
docker run --rm --privileged --name=$CONTAINER \
			--cpuset-cpus="${CPU1}-${CPU4}" \
			-v $(pwd)/data:/data-imported \
			-dt hlefeuvre/osv
docker exec -it $CONTAINER cp /data-imported/nginx.conf \
		   /root/osv/apps/nginx/patches/nginx.conf
docker exec -it $CONTAINER sed -i -e "s/\/usr\/share\/nginx\/html/html/" \
		   /root/osv/apps/nginx/patches/nginx.conf
docker exec -it $CONTAINER bash -c "cd /root/osv &&./scripts/build \
					-j4 fs=ramfs image=nginx"
mkdir -p ${IMAGES}/osv/root/
docker cp ${CONTAINER}:/root/osv/ ${IMAGES}/osv/root/
docker container stop $CONTAINER
docker rm -f $CONTAINER

cp ${IMAGES}/osv/root/osv/build/release/usr.img ${IMAGES}/osv-qemu.img

# ========================================================================
# Generate Rump VM image
# ========================================================================

CONTAINER=rump-tmp
docker pull hlefeuvre/rump
docker run --rm --privileged --name=$CONTAINER \
			--cpuset-cpus="${CPU1}-${CPU4}" \
			-v $(pwd)/data:/data-imported \
			-dt hlefeuvre/rump
docker exec -it $CONTAINER bash -c \
		"cd /root/rumprun-packages && git apply /data-imported/rump.patch"
docker exec -it $CONTAINER cp /data-imported/nginx.conf \
		   /root/rumprun-packages/nginx/images/data/conf/nginx.conf
docker exec -it $CONTAINER sed -i -e "s/\/usr\/share\/nginx\/html/\/data\/www/" \
		   /root/rumprun-packages/nginx/images/data/conf/nginx.conf
docker exec -it $CONTAINER sed -i -e "s/#user  nobody;/user daemon daemon;/" \
		   /root/rumprun-packages/nginx/images/data/conf/nginx.conf
docker exec -it $CONTAINER bash -c \
		". /root/rumprun/obj-amd64-hw/config-PATH.sh && \
		 cd /root/rumprun-packages/nginx && make -j"
docker exec -it $CONTAINER bash -c \
		". /root/rumprun/obj-amd64-hw/config-PATH.sh && \
		cd /root/rumprun-packages/nginx && \
		/root/rumprun/rumprun/bin/rumprun-bake hw_virtio \
		bin/nginx.img ./bin/nginx"
mkdir -p ${IMAGES}/rump/root/
docker cp ${CONTAINER}:/root/rumprun/ ${IMAGES}/rump/root/
docker cp ${CONTAINER}:/root/rumprun-packages/ ${IMAGES}/rump/root/
docker container stop $CONTAINER
docker rm -f $CONTAINER

cp ${IMAGES}/rump/root/rumprun-packages/nginx/bin/nginx.img ${IMAGES}/rump-qemu.img
cp ${IMAGES}/rump/root/rumprun-packages/nginx/images/data.iso ${IMAGES}/rump-qemu.iso
chmod u+x ${IMAGES}/rump/root/rumprun/app-tools/rumprun

# ========================================================================
# Generate Rump VM image
# ========================================================================

./genimage-mirage-solo5-httpreply.sh
