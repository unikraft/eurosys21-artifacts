#!/bin/sh
sudo sh -c \
    "echo performance > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor"
taskset -c 3 qemu-system-x86_64 -cpu host -enable-kvm -nographic -vga none -device isa-debug-exit -fsdev local,id=myid,path=test,security_model=none -device virtio-9p-pci,fsdev=myid,mount_tag=test,disable-modern=on,disable-legacy=off -kernel ./build/lib-helloworld_kvm-x86_64 -append "$1 $2"
#qemu-system-x86_64 -cpu host -enable-kvm -nographic -vga none -device isa-debug-exit -fsdev local,id=myid,path=test,security_model=none -netdev tap,id=guest0,vhost=on,script=no -device virtio-9p-pci,fsdev=myid,mount_tag=test,disable-modern=on,disable-legacy=off -kernel ./build/lib-helloworld_kvm-x86_64 -append "$1 $2"
