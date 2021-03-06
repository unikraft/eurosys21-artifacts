#!/bin/bash
if [ $( id -u ) != 0 ]; then
	echo "Try to run as root..." 1>&2
	exec sudo "$0" "$@"
fi

TAPNAME="tap99"
BRNAME="testbr0"

set -x

# setup experiment bridge
ip tuntap add dev $TAPNAME mode tap
ip link add $BRNAME type bridge
ip link set master $BRNAME dev $TAPNAME
ip link set promisc on dev $BRNAME
ip link set promisc on dev $TAPNAME
ip addr add 172.17.0.113/24 dev $BRNAME
ip link set dev $BRNAME up
ip link set dev $TAPNAME up

# execute VM
qemu-system-x86_64 \
	-cpu host,+invtsc \
	-enable-kvm \
	-m 1G \
	-nodefaults \
	-no-acpi \
	-display none \
	-serial stdio \
	-object memory-backend-file,id=mem,size=1G,mem-path=/mnt/huge1G/test,share=on \
	-numa node,memdev=mem \
	-mem-prealloc \
	-netdev tap,ifname=$TAPNAME,id=testtap0,vhost=on,vhostforce=on,vhostfd=4,script=no,downscript=no 4<>/dev/vhost-net \
	-device virtio-net-pci,netdev=testtap0,addr=0x4,ioeventfd=on,guest_csum=off,gso=off \
	-kernel build/uk_test_suite_kvm-x86_64 \
	| tee results.txt

# destroy network setup
ip link set dev $BRNAME down
ip link set dev $TAPNAME down
ip tuntap del dev $TAPNAME mode tap
ip link del $BRNAME

exit $?
