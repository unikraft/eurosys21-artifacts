echo "Building the unikraft image with dpdk"
cd unikraft/uk_test_suite/ && make fetch
cd build/libukdpdkbuild/origin/
rm -rf dpdk-19.08
ln -s ../../../../../../aux/lib_src/dpdk/ dpdk-19.08
cd ../../..
make -j12
cd ../../
