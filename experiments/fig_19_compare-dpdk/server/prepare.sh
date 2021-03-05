cd unikraft/uk_test_suite/ && make fetch
cd build/libukdpdkbuild/origin/
rm -rf dpdk-19.08
ln -s ~/Unikraft_dpdk/lib_src/dpdk/ dpdk-19.08
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

