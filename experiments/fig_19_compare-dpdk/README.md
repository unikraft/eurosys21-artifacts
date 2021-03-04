# Server
In this scenario, the naming is a little wrong because the server will be genrating traffic. 



# How to run
## Server
The server is running un uktut1. For this experiment we change `TXONLY_DEF_PACKET_LEN` from `dpdk/app/test-pmd/testpmd.h`.

The values:
* 64
* 128
* 256
* 512
* 1024
* 1500

### Unikraft
To run the unikraft generating code
```
cd server
./run_vhost.sh
```

In another terminal:
```
cd server/unikraft/uk_test_suite/
make
./run.sh
```
### Linux
We will be using a linux VM with dpdk installed.

```
cd server
./preparse.sh
./run_vhost
```

In another terminal
```
cd server/linux
./run_linux.sh
```

## Client

The client is running on uktut2.

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

After 10 seconds or so:
```
show port stats all
```

## Cleaning
Run this command on both client and server
`./clean.sh`
