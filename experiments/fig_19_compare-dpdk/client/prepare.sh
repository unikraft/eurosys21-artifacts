sudo modprobe ixgbe
sudo modprobe uio_pci_generic
sudo python3 ./../aux/dpdk/usertools/dpdk-devbind.py --bind=uio_pci_generic enp1s0f1

