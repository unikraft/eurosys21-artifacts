# Experiment description
We have 1 client that is sending udp packets on one machine.

On the second machine we run 4 different server implementations. We use dpdk vhost for all of the VMs.
* `unikraft netdev` we run a udp sever over the rawnetdev.
* `unikraft lwip` we run a udp server over the socket api with with lwip a a network stack
* `linux vm` we run a udp server over the socket API in a linux vm
* `linux dpdk` we run a udp server over dpdk in a linux vm


The results are stored in the client
# Script description
* `run_vhost.sh` runs the dpdk vhost
* `build_all.sh` builds all the necessary binaries
* `udp_echo.sh` parse the output from the client to get the number of packets per second

# Requirements
* meson 0.54.0

Run prepare.sh from the root folder of the experiment on both server and client.

# Server
The server is running on uktut1. We first run the server and next the client. Each folder from the server directory represents an experiment(e.g. unikraft raw netdev, lwip over unikraft etc.). To run an experiment we do the following:

```
cd server
./prepare.sh # only one needed
```
We run the vhost in one terminal
```
./run_vhost.sh
```
We run the server in another terminal

## Raw netdev
```
cd unikraft_raw/uk_test_suite
./run.sh
```


# Client
The client is running on uktut2. The client is a dpdk application that generates traffic. Run the client after starting one of the servers

```
cd client
./run_client
```

In the test-pmd interactive shell we run the following:
```
```


Connection details
client                server
uktut2      <-------> uktut1
enp1s0f0 	      enp1s0f1
