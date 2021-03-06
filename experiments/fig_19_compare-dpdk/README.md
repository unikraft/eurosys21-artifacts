# TX throughput experiment (Unikraft v.s. DPDK)

This experiment evaluates the performance of Unikraft's `uknetdev` API.  To
this end we wrote a simple application that sends as many packets as
possible from one machine (with varying packet sizes), and measure the
achieved throughput on a different machine running a DPDK `testpmd`
application.

# How to run

## Server

The server is running on the first machine (in the AE testbed, this is
`uktut1`). For this experiment we change `TXONLY_DEF_PACKET_LEN` from
`dpdk/app/test-pmd/testpmd.h` in order to vary packet sizes.

Following sizes are explored: 64, 128, 256, 512, 1024, and 1500.

### Unikraft vhost user

First, prepare the environment:
```
./prepare.sh
```

To run the unikraft generating code
```
cd server
./run_vhost.sh
```

In the vhost interactive shell:
```
port stop all
set fwd mac
port start all
start
```

In another terminal:
```
cd server
./prepare.sh
cd unikraft/uk_test_suite/
./run_vhost_user.sh
```

We'll see an output of 10 iteration like:
```
******************** ITERATION: 2 ******************
The count of packet to transmitted/NR of packet sent to virtio per sec/Total iteration
12432480/1216/12431264/0/558524000/8921195
12450784/256/12450528/0/559758560/8940771
12440448/0/12440448/0/560478464/8951858
12445632/0/12445632/0/560391936/8950587
12435296/0/12435296/0/562330272/8980712
The count of packets : 123644384
The latency of pkt alloc/pkt gen/ pkt send
247069368/1033896958/1370259576
246913586/1035093974/1368389088
247222812/1033868874/1367967382
246885880/1033856334/1369072622
247798612/1031452360/1367364560
```
The first column is the result, the number of packet/s per second transmited.

Note: If the value of `TXONLY_DEF_PACKET_LEN` is changed, we have to
rebuild Unikraft. To do this we run
```
rm -rf unikraft/uk_test_suite/
./prepare.sh
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
```

## Cleaning

Run this command on both client and server

```
./clean.sh
```
