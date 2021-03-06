sudo ip a  flush dev  enp1s0f1
sudo ip a a 192.168.4.1/24 dev enp1s0f1
sudo ip link set enp1s0f1 up

