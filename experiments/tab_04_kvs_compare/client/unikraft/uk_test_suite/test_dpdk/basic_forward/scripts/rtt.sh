#!/bin/bash
paste -d, <(cat ./dpdk_test_rtt/anylayout/rtt_72B_256b | grep -A10 latency | grep ^[1-9] | awk -F ":" '{print $1}' | awk -F "(" '{print $1}') <(cat ./dpdk_test_rtt/anylayout/rtt_72B_256b | grep -A10 latency | grep ^[1-9] | awk -F ":" '{print $2}' | awk -F "/" '{print $1}') > ./dpdk_test_rtt/anylayout/rtt_72B_256b_latency_ds

cat ./dpdk_test_rtt/anylayout/rtt_72B_256b_latency_ds | awk --bignum -F',' '{printf "%d\n", $1/$2}' | sort -n > ./dpdk_test_rtt/anylayout/rtt_72B_256b_rttlatency_sorted
