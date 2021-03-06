# UDP key-value store performance (Unikraft v.s. DPDK)

This experiment demonstrates the potential of specialization for
network applications. To this end we implement a UDP-based in-memory
key-value store using the `recvmsg`/`sendmsg` syscalls and apply
different levels of specialization (batching, raw uknetdev, DPDK) both
on Linux and Unikraft. We demonstrate that specialized unikernels can
achieve or exceed the performance of DPDK with less resources.

## How to run

This experiment leverages a two-machine setup. The first machine (in
the AE testbed `uktut1`) sends UDP packets with different degrees of
specialization (Linux/Unikraft). The second machine (in the AE testbed
`uktut2`) simply receives them. We use dpdk vhost for all of the VMs.

The different servers (representing the different degrees of
specialization) run on the first machine are:
* `unikraft netdev` we run a udp sever over the rawnetdev.
* `unikraft lwip` we run a udp server over the socket api with with lwip a a network stack
* `linux vm` we run a udp server over the socket API in a linux vm
* `linux dpdk` we run a udp server over dpdk in a linux vm

The results are stored in the client.

## Notes

Please note that for reproducing, we additionally allowed for user-defined CPU
frequency scaling setup (`intel_pstate=disable`) with the boot configuration.
For instance with Grub (`/etc/default/grub`):

``` bash
GRUB_CMDLINE_LINUX_DEFAULT="isolcpus=2-6 noht intel_iommu=off ipv6.disable=1 intel_pstate=disable"
```

From the isolated 4 CPU cores, we pinned one to the VM, another one to the VMM
(e.g., `qemu-system-x86_64`), and another one to the client tool (e.g., `wrk` or
`redis-benchmark`).  For all experiments, we set the governor to performance,
which can be done generally by:

```
echo "performance" > /sys/devices/system/cpu/cpu$CPU_ID/cpufreq/scaling_governor
```

However, both the pinning and governor settings are handled by the scripts in
this repo (as opposed to the kernel boot parameters, which you will need to take
care of manually).

# Script description

* `run_vhost.sh` runs the dpdk vhost
* `build_all.sh` builds all the necessary binaries
* `udp_echo.sh` parse the output from the client to get the number of packets per second

# Requirements

* meson 0.54.0

Run prepare.sh from the root folder of the experiment on both server and client.

!!!! After each experiment, it is necessary to do a `clean.sh` and take everything from
0 for the next experiment.


# Server

The server is running on uktut1. We first run the server and next the
client. Each folder from the server directory represents an
experiment(e.g. unikraft raw netdev, lwip over unikraft etc.). To run
an experiment we do the following:

We run `./prepare.sh` in the root. Then we run the following:
```
cd server
./prepare.sh 
```

We run the vhost in one terminal
```
./run_vhost.sh
```
In the vhost terminal we run
```
set fwd mac
start
```

We run the server in another terminal. Depending on which server we want to run. There is a `./run.sh`. 

## Raw netdev
```
cd unikraft_raw/uk_test_suite
./run.sh
```

## Linux dpdk
```
cd server/linux_dpdk/
run_server.sh
```

## Linux single
```
cd server/linux_single/
./run.sh
```

# Client
The client is running on uktut2. The client is a dpdk application that generates traffic. Run the client after starting one of the servers.

We run `./prepare.sh` in the root. Then we run the following:
```
cd client
./prepare.sh
```

In the test-pmd interactive shell we run the following:
```
set fwd mac
start
```

To run the client we add:
```
cd unikraft/uk_test_suite
./run.sh
```

The client will output the results every second:
```
******iteration: 95*********************                
The latency, iteration, count of packet to received/transmitted per sec
458197340378,877788,6514181,0                           
The count of packet to received: 617872698                             
The latency of pkt recv/0 pkt recv/pkt process/ pkt free
1052610566/640142000/1141215506/83314356  
```
We're interested in the third value in the second row:6514181. That 
is the number of packets/s. The client can be kept up while changing servers.

# Connection details
client                server
uktut2      <-------> uktut1
enp1s0f0 	      enp1s0f1
