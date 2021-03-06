sudo python3 aux/dpdk/usertools/dpdk-devbind.py -u 0000:01:00.0
sudo python3 aux/dpdk/usertools/dpdk-devbind.py -b ixgbe 0000:01:00.0 0000:01:00.1
rm -rf aux/lib_src
rm -rf aux/dpdk2
rm -rf server/unikraft/uk_test_suite/build
rm -rf client/unikraft/uk_test_suite/build
