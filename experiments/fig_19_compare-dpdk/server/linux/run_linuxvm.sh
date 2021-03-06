#!/bin/bash
if [ $( id -u ) != 0 ]; then
	echo "Try to run as root..." 1>&2
	exec sudo "$0" "$@"
fi

QCOW="../../aux/debian.qcow2"
DPDKVHOST="/root/dpdk/build/app/vhost-net"
MEMPATH="/mnt/huge1G/test"
TAPNAME="tap88"
BRNAME="expbr0"

set -x

# setup mgmt bridge
ip tuntap add dev $TAPNAME mode tap
ip link add $BRNAME type bridge
ip link set master $BRNAME dev $TAPNAME
ip link set promisc on dev $BRNAME
ip link set promisc on dev $TAPNAME
ip addr add 172.18.0.254/24 dev $BRNAME
ip link set dev $BRNAME up
ip link set dev $TAPNAME up

# run VM
taskset -c 4,5 qemu-system-x86_64 \
	-m 6G \
	-cpu host,+invtsc \
	-smp cpus=2 \
	-enable-kvm \
	-mem-prealloc \
	\
	-netdev tap,ifname=$TAPNAME,id=mgm0,script=no,downscript=no \
	-device virtio-net-pci,netdev=mgm0 \
	\
	-chardev "socket,id=char0,path=$DPDKVHOST" \
	-netdev vhost-user,id=testtap1,chardev=char0,vhostforce \
	-device virtio-net-pci,netdev=testtap1,id=dpdknet0,rx_queue_size=1024,tx_queue_size=1024 \
	\
	-hda "$QCOW" \
	-display curses \
	-boot c
RET=$?

# destroy network setup
ip link set dev $BRNAME down
ip link set dev $TAPNAME down
ip tuntap del dev $TAPNAME mode tap
ip link del $BRNAME

exit $RET
