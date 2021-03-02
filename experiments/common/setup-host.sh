#!/bin/bash

# verbose output
set -x

modprobe kvm_intel
sysctl -w net.ipv4.conf.all.forwarding=1

sysctl -w net.ipv4.netfilter.ip_conntrack_max=99999999
sysctl -w net.nf_conntrack_max=99999999
sysctl -w net.netfilter.nf_conntrack_max=99999999

sysctl -w net.ipv4.neigh.default.gc_thresh1=1024
sysctl -w net.ipv4.neigh.default.gc_thresh2=2048
sysctl -w net.ipv4.neigh.default.gc_thresh3=4096

echo 1024 > /proc/sys/net/core/somaxconn
echo never > /sys/kernel/mm/transparent_hugepage/enabled

# CPU frequency
cpupower frequency-set -g performance
