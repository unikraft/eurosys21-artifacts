echo "Building the unikraft image with dpdk"
cd unikraft_raw/uk_test_suite/ && make fetch
cd build/libukdpdkbuild/origin/
rm -rf dpdk-19.08
ln -s ../../../../../../aux/lib_src/dpdk/ dpdk-19.08
cd ../../..
make -j12
cd ../../


echo "Getting the Linux VM with DPDK"

FILE=../aux/debian.qcow2
if test -f "$FILE"; then
    echo "$FILE exists."
else
    cd ../aux && wget http://vmx.cs.pub.ro/~vmchecker/debian.qcow2
fi

cd unikraft_lwip/
git clone https://github.com/unikraft/unikraft
mkdir libs
cd libs
git clone https://github.com/unikraft/lib-lwip
