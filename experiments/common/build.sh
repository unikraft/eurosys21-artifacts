#!/bin/bash

BUILDDIR=../
LUPINEDIR=${BUILDDIR}/.lupine

build_lupine() {
    mkdir -p ${LUPINEDIR}

    if [ ! -d "${LUPINEDIR}/Lupine-Linux" ]; then
	    pushd ${LUPINEDIR}
	    git clone https://github.com/hlef/Lupine-Linux.git
	    pushd Lupine-Linux
	    git checkout b9dc99bbd09180b0a3548583d58f9c003d4576e8
	    git submodule update --init
	    make build-env-image
	    pushd load_entropy
	    make
	    popd
	    popd
	    popd
    fi

    #rm -rf ${LUPINEDIR}/Lupine-Linux/kernelbuild
    if [ ! -d "${LUPINEDIR}/Lupine-Linux/kernelbuild" ]; then
	pushd ${LUPINEDIR}/Lupine-Linux
	# build helloworld as well
	sed -i -e "s/redis nginx/redis nginx hello-world/" scripts/build-kernels.sh

	# build qemu/kvm version
	cp configs/lupine-djw-kml.config configs/lupine-djw-kml-qemu.config
	echo "CONFIG_PCI=y" >> configs/lupine-djw-kml-qemu.config
	echo "CONFIG_PCI=y" >> configs/microvm.config
	echo "CONFIG_VIRTIO_BLK_SCSI=y" >> configs/lupine-djw-kml-qemu.config
	echo "CONFIG_VIRTIO_BLK_SCSI=y" >> configs/microvm.config
	echo "CONFIG_VIRTIO_PCI_LEGACY=y" >> configs/lupine-djw-kml-qemu.config
	echo "CONFIG_VIRTIO_PCI_LEGACY=y" >> configs/microvm.config
	echo "CONFIG_VIRTIO_PCI=y" >> configs/lupine-djw-kml-qemu.config
	echo "CONFIG_VIRTIO_PCI=y" >> configs/microvm.config
	echo "CONFIG_VGA_ARB_MAX_GPUS=16" >> configs/microvm.config

	./scripts/build-with-configs.sh configs/lupine-djw-kml-qemu.config \
						configs/apps/nginx.config
	./scripts/build-with-configs.sh configs/lupine-djw-kml-qemu.config \
						configs/apps/redis.config
	./scripts/build-with-configs.sh nopatch configs/microvm.config \
						configs/apps/nginx.config
	./scripts/build-with-configs.sh nopatch configs/microvm.config \
						configs/apps/redis.config

	# just in case :)
	make build-env-image

	# build normal lupine kernels
	# TODO it would be nice to build this with GCC 6.3.0 to be absolutely fair
	./scripts/build-kernels.sh

	# idempotence
	git checkout scripts/build-kernels.sh
	popd
    fi
}

unikraft_eurosys21_build() {
    CONTAINER=uk-tmp-nginx
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
		${3}/unikraft+${2}.kernel
    docker container stop $CONTAINER
    docker rm -f $CONTAINER
    sleep 6
}

