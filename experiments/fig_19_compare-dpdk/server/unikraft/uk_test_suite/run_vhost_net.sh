#!/bin/bash
if [ $( id -u ) != 0 ]; then
	echo "Try to run as root..." 1>&2
	exec sudo "$0" "$@"
fi

TAPNAME="tap99"
BRNAME="testbr0"

set -x

ip link add link enp1s0f1 macvtap2 address 00:22:33:44:55:66 type macvtap mode bridge
ip link set promisc on dev enp1s0f1
ip link set macvtap2 up
ip link set dev enp1s0f1 up
my_fd=$(cat /sys/class/net/macvtap2/ifindex)
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
	-netdev tap,fd=5,id=testtap0,vhost=on,vhostforce=on 4<>/dev/vhost-net 5<>/dev/tap$my_fd\
	-device virtio-net-pci,netdev=testtap0,addr=0x4,ioeventfd=on,guest_csum=off,gso=off \
	-kernel build/uk_test_suite_kvm-x86_64 \
	| tee results.txt
RET=$?

ip link del macvtap2
exit $RET
