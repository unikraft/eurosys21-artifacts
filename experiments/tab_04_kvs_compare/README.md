# Experiment description
We have 1 client that is sending udp packets on one machine.

On the second machine we run 4 different server implementations. We use dpdk vhost for all of the VMs.
* `unikraft netdev` we run a udp sever over the rawnetdev.
* `unikraft lwip` we run a udp server over the socket api with with lwip a a network stack
* `linux vm` we run a udp server over the socket API in a linux vm
* `linux dpdk` we run a udp server over dpdk in a linux vm

# Script description
* `run_vhost.sh` runs the dpdk vhost
* `build_all.sh` builds all the necessary binaries
* `udp_echo.sh` parse the output from the client to get the number of packets per second

# Requirements
* meson 0.54.0

# Server
The server is running on uktut1.
