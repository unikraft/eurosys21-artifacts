gcc server.c -o server
ip a a 172.17.0.114 dev  enp2s0
ip a l set enp2s0 up
./server
