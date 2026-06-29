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
	    popd
	    popd
    fi

    if [ ! -d "${LUPINEDIR}/Lupine-Linux/linux/kernel" ]; then
	    pushd ${LUPINEDIR}/Lupine-Linux
	    rm -rf linux osv
	    git submodule update --init --depth 1

	    # Use archive.debian.org for Jessie repositories since Jessie is EOL
	    cat << 'EOF' > docker/build-env.Dockerfile
FROM debian:jessie-slim
RUN echo "deb http://archive.debian.org/debian jessie main" > /etc/apt/sources.list && \
    echo "deb http://archive.debian.org/debian-security jessie/updates main" >> /etc/apt/sources.list && \
    echo 'Acquire::Check-Valid-Until "false";' > /etc/apt/apt.conf.d/99no-check-valid-until && \
    echo 'Acquire::AllowInsecureRepositories "true";' >> /etc/apt/apt.conf.d/99no-check-valid-until && \
    echo 'APT::Get::AllowUnauthenticated "true";' > /etc/apt/apt.conf.d/99allow-unauthenticated
RUN apt-get update
RUN apt-get -y install --force-yes build-essential bc
RUN apt-get -y install --force-yes libssl-dev
RUN apt-get -y install --force-yes openssl
EOF

	    make build-env-image
	    pushd load_entropy
	    make
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
    unikraft_eurosys21_build_wvmm $1 $2 $3 kvm
}

unikraft_eurosys21_build_wvmm() {
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
    docker cp ${CONTAINER}:/root/workspace/apps/app-${1}/build/app-${1}_${4}-x86_64 \
		${3}/unikraft+${2}.kernel
    # special case: for solo5, also copy hvt
    if [ "$4" = "solo5" ]; then
        docker cp ${CONTAINER}:/root/workspace/apps/app-${1}/build/solo5-hvt \
            ${IMAGES}/solo5_hvt
    fi
    docker container stop $CONTAINER
    docker rm -f $CONTAINER
    sleep 6
}

