#!/bin/bash
if [ $( id -u ) != 0 ]; then
	echo "Try to run as root..." 1>&2
	exec sudo "$0" "$@"
fi

QCOW="../../aux/debian.qcow2"
MEMPATH="/mnt/huge1G/test"
TAPNAME="tap88"
BRNAME="expbr0"

TAPNAME_EXP="tap98"
BRNAME_EXP="testbr0"

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

ip link add link enp1s0f1 macvtap2 address 00:22:33:44:55:66 type macvtap mode bridge
ip link set promisc on dev enp1s0f1
ip link set macvtap2 up
ip link set dev enp1s0f1 up
my_fd=$(cat /sys/class/net/macvtap2/ifindex)

# run VM
taskset -c 4,5 qemu-system-x86_64 \
	-m 6G \
	-cpu host,+invtsc \
	-smp cpus=2 \
	-enable-kvm \
	-object memory-backend-file,id=mem,size=6144M,mem-path=/mnt/linux-huge,share=on -numa node,memdev=mem \
	\
	-netdev tap,ifname=$TAPNAME,id=mgm0,script=no,downscript=no \
	-device virtio-net-pci,netdev=mgm0 \
	\
	-netdev tap,fd=5,id=testtap1,vhost=on,vhostforce=on 4<>/dev/vhost-net 5<>/dev/tap$my_fd\
	-device virtio-net-pci,netdev=testtap1,ioeventfd=on,guest_csum=off,gso=off \
	\
	-hda "$QCOW" \
	-display curses \
	-boot c
RET=$?

# destroy network setup
ip link del macvtap2

ip link set dev $BRNAME down
ip link set dev $TAPNAME down
ip tuntap del dev $TAPNAME mode tap
ip link del $BRNAME

exit $RET
