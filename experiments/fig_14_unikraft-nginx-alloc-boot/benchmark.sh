#!/bin/bash

# verbose output
set -x

source ../common/set-cpus.sh
source ../common/qemu.sh

IMAGES=$(pwd)/images
QEMU_GUEST=qemu-guest
MEM=1024

kill_qemu

for alloc in "tinyalloc" "tlsf" "buddy" "mimalloc" "region";
do
	mkdir -p rawdata/${MEM}M
	LOG=rawdata/${MEM}M/${alloc}-`date '+%d%m%Y%H%M%S'`.txt
	touch $LOG

	for j in {1..10}
	do
		echo "running batch ${alloc}, ${j}/10" >> $LOG

		# start nginx server (daemon)
		taskset -c ${CPU1} ${QEMU_GUEST} -i nginx.cpio \
			-k ${IMAGES}/unikraft+${alloc}.kernel \
			-m ${MEM} -g 1234 -P -p ${CPU2} -x

		# start gdb session
		gdb --eval-command="target remote :1234" \
			-ex "set confirm off" \
			-ex "set pagination off" \
			-ex "hbreak ukplat_terminate" \
			-ex "c" \
			-ex "disconnect" \
			-ex "set arch i386:x86-64:intel" \
			-ex "tar remote localhost:1234" \
			-ex "uk trace" \
			-ex "quit" \
			${IMAGES}/unikraft+${alloc}.kernel >> ${LOG}
		kill_qemu
	done

	addresses=`cat $LOG | awk -e '$0 ~ /trace_boot/ {print $3}' | \
			sed -r '/^\s*$/d' | sed -r "s/\(\)//g"`

	for addr in $addresses;
	do
		fname=`addr2line -e ${IMAGES}/unikraft+${alloc}.kernel \
				-fp $addr | awk -e '$0 {print $1}'`
		sed -i "s/$addr/$fname/g" $LOG
		LC_ALL=C tr -dc '\0-\177' <$LOG >newfile &&
		mv newfile $LOG
	done
done

./parse.py
