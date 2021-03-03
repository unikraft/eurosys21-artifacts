#!/bin/bash

set -x

modprobe tun

BASEIP=172.190.0

# start a dnsmasq server and echo its PID
function run_dhcp {
    dnsmasq -d \
        --log-queries \
        --bind-dynamic \
        --interface=$1 \
        --listen-addr=${2}.1 \
        --dhcp-range=${2}.2,${2}.254,255.255.255.0,12h &> $(pwd)/dnsmasq.log &
    echo $!
}

# take dnsmasq PID as $1
function kill_dhcp {
    kill -9 $1
    rm dnsmasq.log
}

[[ -z "${CONN_LATENCY}" ]] && CONN_LATENCY=0

function create_bridge {
    brctl addbr $1 || true
    tc qdisc add dev $1 root netem delay ${CONN_LATENCY}ms
    ifconfig $1 ${2}.1
}

function delete_bridge {
    ifconfig $1 down
    brctl delbr $1
}

function create_tap {
    if ! ip link show $1 &> /dev/null; then
        sudo ip tuntap add mode tap $1
        sudo ip addr add ${2}.1/24 dev $1
	tc qdisc add dev $1 root netem delay ${CONN_LATENCY}ms
        sudo ip link set $1 up
        echo 1 | sudo tee /proc/sys/net/ipv4/ip_forward > /dev/null
	echo 1 > /proc/sys/net/ipv4/conf/$NETIF/proxy_arp
        sudo iptables -t nat -A POSTROUTING -o bond1 -j MASQUERADE
        sudo iptables -I FORWARD 1 -i $1 -j ACCEPT
        sudo iptables -I FORWARD 1 -o $1 -m state --state RELATED,ESTABLISHED -j ACCEPT
    fi
}

function delete_tap {
    sudo ip link del $1
}
