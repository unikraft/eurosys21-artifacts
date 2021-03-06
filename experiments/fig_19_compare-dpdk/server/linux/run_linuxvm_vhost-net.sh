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

# setup experiment bridge
ip tuntap add dev $TAPNAME_EXP mode tap
ip link add $BRNAME_EXP type bridge
ip link set master $BRNAME_EXP dev $TAPNAME_EXP
ip link set promisc on dev $BRNAME_EXP
ip link set promisc on dev $TAPNAME_EXP
ip addr add 172.17.0.113/24 dev $BRNAME_EXP
ip link set dev $BRNAME_EXP up
ip link set dev $TAPNAME_EXP up

# run VM
taskset -c 4,5 qemu-system-x86_64 \
	-m 6G \
	-cpu host,+invtsc \
	-smp cpus=2 \
	-enable-kvm \
	-object memory-backend-file,id=mem,size=6144M,mem-path=${MEMPATH},share=on \
	-numa node,memdev=mem \
	-mem-prealloc \
	\
	-netdev tap,ifname=$TAPNAME,id=mgm0,script=no,downscript=no \
	-device virtio-net-pci,netdev=mgm0 \
	\
	-netdev tap,ifname=$TAPNAME_EXP,id=testtap1,vhost=on,vhostforce=on,vhostfd=4,script=no,downscript=no 4<>/dev/vhost-net \
	-device virtio-net-pci,netdev=testtap1,ioeventfd=on,guest_csum=off,gso=off \
	\
	-hda "$QCOW" \
	-display curses \
	-boot c
RET=$?

#	-mem-path /dev/ \

##	-object memory-backend-file,id=mem,size=1G,mem-path=/mnt/huge1G/test,share=on \
##	-numa node,memdev=mem \

# destroy network setup
ip link set dev $BRNAME_EXP down
ip link set dev $TAPNAME_EXP down
ip tuntap del dev $TAPNAME_EXP mode tap
ip link del $BRNAME_EXP

ip link set dev $BRNAME down
ip link set dev $TAPNAME down
ip tuntap del dev $TAPNAME mode tap
ip link del $BRNAME

exit $RET
