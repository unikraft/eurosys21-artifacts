./build.sh
sudo arp -s 172.18.0.4 a0:36:9f:52:29:D0
sudo ip a a 172.18.0.114 dev enp1s0f1
sudo ifconfig enp1s0f1 hw ether 52:54:00:12:34:57
sudo ip l set enp1s0f1 up
sudo iptables -P INPUT ACCEPT
sudo iptables -P OUTPUT ACCEPT
sudo iptables -P FORWARD ACCEPT
sudo iptables -F
sudo sysctl -w net.core.rmem_max=26214400
sudo sysctl -w net.core.rmem_default=26214400
./udpreceiver1
