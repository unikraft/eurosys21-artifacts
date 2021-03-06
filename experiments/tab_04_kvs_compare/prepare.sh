# get dpdk
cd aux
wget http://vmx.cs.pub.ro/~vmchecker/lib_src.zip
unzip lib_src
rm lib_src.zip

wget https://fast.dpdk.org/rel/dpdk-19.11.6.tar.xz
tar xJf dpdk-19.11.6.tar.xz
rm dpdk-19.11.6.tar.xz
mv dpdk-stable-19.11.6/ dpdk2
cd dpdk2
meson build
cd build
ninja

cd ../../
wget http://vmx.cs.pub.ro/~vmchecker/dpdk3.zip
unzip dpdk3
cd dpdk3
meson build
cd build
ninja

