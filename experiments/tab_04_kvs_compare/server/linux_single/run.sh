gcc server.c -o server
sudo arp -s 172.17.0.4 a0:36:9f:52:29:D0
sudo ip a a 172.17.0.114 dev enp1s0f1
sudo ip l set enp1s0f1 up
./server
