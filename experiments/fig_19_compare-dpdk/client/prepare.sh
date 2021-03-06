sudo modprobe ixgbe
sudo modprobe uio_pci_generic
sudo ip a flush enp1s0f0
sudo python3 ./../aux/dpdk/usertools/dpdk-devbind.py --bind=uio_pci_generic enp1s0f0

