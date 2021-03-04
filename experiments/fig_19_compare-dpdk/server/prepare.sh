cd unikraft/uk_test_suite/ && make
cd ../..


echo "Getting the Linux VM with DPDK"

FILE=../aux/debian.qcow2
if test -f "$FILE"; then
    echo "$FILE exists."
else
    cd ../aux && wget http://vmx.cs.pub.ro/~vmchecker/debian.qcow2
fi

