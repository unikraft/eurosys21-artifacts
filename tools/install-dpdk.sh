#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
#
# DPDK build & installation tool
#
# @author Alexander Jung <a.jung@lancs.ac.uk>

WORKDIR=$(dirname $0)/..
. $WORKDIR/support/common.sh

# Influential environmental variables
DPDK_VERSION=${DPDK_VERSION:-v17.05}
RTE_SDK=${RTE_SDK:-$WORKDIR/build/dpdk}
RTE_TARGET=${RTE_TARGET:-x86_64-native-linuxapp-gcc}
NET_IF_NAME=${NET_IF_NAME:-enp0s8}
HUGEPAGE_MOUNT=${HUGEPAGE_MOUNT:-/mnt/huge}

# Program arguments
ONLY_PREPARE=n

_help() {
    cat <<EOF
$0 - Build and install DPDK

Usage:
  $0 [OPTIONS]

Options:
     --only-preapre    Only prepare Lupine in build directory.
  -v --verbose         Be verbose.
  -h --help            Show this help menu.

Influential Environmental Variables
  DPDK_VERSION         DPDK version to use
                         (default: $DPDK_VERSION).
  RTE_SDK              Path to the DPDK dir
                         (default: $RTE_SDK).
  RTE_TARGET           Target of build process
                         (default: $RTE_TARGET).
  NET_IF_NAME          Name of network interface provisioned for
                         DPDK to bind (default: $NET_IF_NAME).
  HUGEPAGE_MOUNT       Hugepages mount point
                        (default: $HUGEPAGE_MOUNT).
EOF
}

# Parse flag arguments
for i in "$@"; do
  case $i in
    --only-prepare)
      ONLY_PREPARE=y; shift;;
    --no-docker)
      NO_DOCKER=y; shift;;
    -h|--help)
      _help; exit 0;;
  esac
done

root_or_die

# Dependency management
function install_dependencies() {
  local DISTRO=$(lsb_release -c -s)

  case $DISTRO in
    buster)
      apt-get update
      apt-get install -y \
        git \
        clang \
        doxygen \
        libhugetlbfs-bin \
        build-essential 
      ;;
    *)
      log_err "This script is not yet compatible with $DISTRO"
      exit 1
      ;;
  esac
}

# Configure hugepages
# You can later check if this change was successful with `cat /proc/meminfo`
# Hugepages setup should be done as early as possible after boot
function set_huge_pages() {
  echo 1024 | sudo tee /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages
  mkdir ${HUGEPAGE_MOUNT}
  mount -t hugetlbfs nodev ${HUGEPAGE_MOUNT}

  # Set Hugepages in /etc/fstab so they persist across reboots
  echo "hugetlbfs ${HUGEPAGE_MOUNT} hugetlbfs rw,mode=0777 0 0" | sudo tee -a /etc/fstab
}

# Get code from Git repo
function get_dpdk() {
  git clone http://dpdk.org/git/dpdk ${RTE_SDK}

  pushd ${RTE_SDK}
    # Checkout desired version, if specified
    git checkout "${DPDK_VERSION:-master}"
  
  popd
}

# Config and build
function build_dpdk() {
  pushd ${RTE_SDK}
    make config T=${RTE_TARGET}
    sed -ri 's,(PMD_PCAP=).*,\1y,' build/.config
    make

    ln -sf ${RTE_SDK}/build ${RTE_SDK}/${RTE_TARGET}
  popd
}

# Install kernel modules
function install_lkm() {
  modprobe uio
  insmod ${RTE_SDK}/build/kmod/igb_uio.ko

  # Make uio and igb_uio installations persist across reboots
  sudo ln -sf ${RTE_SDK}/build/kmod/igb_uio.ko /lib/modules/`uname -r`
  sudo depmod -a
  echo "uio" | sudo tee -a /etc/modules
  echo "igb_uio" | sudo tee -a /etc/modules
}

# Bind secondary network adapter
function bind_interface() {
  ifconfig ${NET_IF_NAME} down
  ${RTE_SDK}/usertools/dpdk-devbind.py --bind=igb_uio ${NET_IF_NAME}
}

# Install dpdk...
install_dependencies

if [[ $ONLY_PREPARE != 'y' ]]; then
  set_huge_pages
fi

get_dpdk
build_dpdk

if [[ $ONLY_PREPARE != 'y' ]]; then
  set_huge_pages
  install_lkm
  bind_interface
fi

# echo "export RTE_SDK=${RTE_SDK}" >> ${HOME}/.profile
# echo "export RTE_TARGET=${RTE_TARGET}" >> ${HOME}/.profile
