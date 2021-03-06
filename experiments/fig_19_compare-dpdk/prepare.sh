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

if [[ ! -d /mnt/huge1G ]]; then
  mkdir /mnt/huge1G
  mount -t hugetlbfs -o pagesize=1G none /mnt/huge1G
fi
