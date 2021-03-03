#!/usr/bin/env bash
WORKDIR=$(dirname $0)/..
. $WORKDIR/support/common.sh

# Influential environmental variables
BUILD_DIR=${BUILD_DIR:-$WORKDIR/build}
LUPINE_DIR=${LUPINE_DIR:-$BUILD_DIR/lupine}

# Program arguments
ONLY_PREPARE=n
NO_DOCKER=n

_help() {
    cat <<EOF
$0 - Build Lupine Linux.

Usage:
  $0 [OPTIONS]

Options:
  -d --dir             Path to build Lupine.
     --no-docker       Do not use Docker as the build environment.
     --only-preapre    Only prepare Lupine in build directory.
  -v --verbose         Be verbose.
  -h --help            Show this help menu.

Influential Environmental Variables
  BUILD_DIR            Path to the build directory
                         (default: ./build).
  LUPINE_DIR           Path to Lupine source code
                         (default: ./build/lupine).
EOF
}

# Parse flag arguments
for i in "$@"; do
  case $i in
    -d |--dir )
      LUPINE_DIR=$2; shift 2;;
    --only-prepare)
      ONLY_PREPARE=y; shift;;
    --no-docker)
      NO_DOCKER=y; shift;;
    -h|--help)
      _help; exit 0;;
  esac
done


if [[ ! -d "$LUPINE_DIR" ]]; then
  git clone https://github.com/hlef/Lupine-Linux.git $LUPINE_DIR

  pushd $LUPINE_DIR
    git checkout b9dc99bbd09180b0a3548583d58f9c003d4576e8
    git submodule update --init

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
  popd
fi

if [[ $ONLY_PREPARE == 'y' || $NO_DOCKER != 'y' ]]; then
  make -C $LUPINE_DIR build-env-image
fi

if [[ $ONLY_PREPARE != 'y' && ! -d "$LUPINE_DIR/kernelbuild" ]]; then
  pushd $LUPINE_DIR

    pushd load_entropy
      make
    popd

    # build normal lupine kernels
    # TODO it would be nice to build this with GCC 6.3.0 to be absolutely fair
    ./scripts/build-kernels.sh

    # idempotence
    git checkout scripts/build-kernels.sh
  popd
fi
