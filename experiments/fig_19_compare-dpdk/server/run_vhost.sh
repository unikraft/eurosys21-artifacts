sudo modprobe ixgbe
sudo modprobe uio_pci_generic
sudo python3 ../aux/dpdk/usertools/dpdk-devbind.py --bind=uio_pci_generic enp1s0f1
sudo ../aux/dpdk/build/app/testpmd -l 2,3 -n 4 --vdev 'eth_vhost0,iface=vhost-net,queues=1' -- -i --txd=1024 --rxd=1024 --nb-cores=1 --eth-peer=0,52:54:00:12:34:57 --eth-peer=1,52:54:00:12:34:57
