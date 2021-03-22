# TX throughput experiment (Unikraft v.s. DPDK)

<img align="right" src="../../plots/fig_19_compare-dpdk.svg" width="300" />

This experiment evaluates the performance of Unikraft's `uknetdev` API.  To
this end we wrote a simple application that sends as many packets as
possible from one machine (with varying packet sizes), and measure the
achieved throughput on a different machine running a DPDK `testpmd`
application.

# How to run

## Server

The server is running on the first machine (in the AE testbed, this is
`uktut1`). For this experiment we change `TXONLY_DEF_PACKET_LEN` from
`dpdk/app/test-pmd/testpmd.h` in linux dpdk and test_dpdk/basic_forward/pktgen.h
in unikraft in order to vary packet sizes.

Following sizes are explored: 64, 128, 256, 512, 1024, and 1500.

### unikraft vhost user

First, prepare the environment and build the Unikraft unikernel:
```
./prepare.sh
```

Then, we start the vhost:
```
cd server
./run_vhost.sh
```

This is going to pop an interactive shell. In this shell, enter the following
commands:
```
port stop all
set fwd mac
port start all
start
```

Finally, in another terminal, start the Unikraft unikernel that uses the
vhost-user port:
```
cd server
./prepare.sh
cd unikraft/uk_test_suite/
./run_vhost_user.sh
```


Note: If the value of `TXONLY_DEF_PACKET_LEN` is changed, Unikraft has to be rebuilt.
To do this, run:
```
make
```

### unikraft vhost net
Similar with the vhost user, but without dpdk:

First, prepare the environment and build the Unikraft unikernel:
```
./prepare.sh
```

```
cd server
./prepare.sh
cd unikraft/uk_test_suite/
./run_vhost_net.sh
```

### Linux

For this experiment, we use a linux VM with DPDK preinstalled.

## Client

The client is running on a second machine, connected to the first machine
with a 10Gb/s ethernet cable. In the AE testbed, this corresponds to
`uktut2`.

```
./build.sh
cd client
./prepare.sh
./run_client.sh
```

We now enter the following commands in the interactive test-pmd shell:

```
set fwd rxonly
start

To see the results:
```
show ports stats all
```

Example output, the result is 14204107.
```
testpmd> show port stats all

  ######################## NIC statistics for port 0  ########################
  RX-packets: 3064735220 RX-missed: 0          RX-bytes:  253701795528
  RX-errors: 0
  RX-nombuf:  0
  TX-packets: 0          TX-errors: 0          TX-bytes:  0

  Throughput (since last show)
  Rx-pps:     14204107
  Tx-pps:            0
  ############################################################################
```


## Cleaning

Run this command on both client and server

```
./clean.sh
```

## Notes

We make use of huge pages for this experiment, which has the following Linux
kernel parameters:

```
default_hugepagesz=1G hugepagesz=1G hugepages=4
```
