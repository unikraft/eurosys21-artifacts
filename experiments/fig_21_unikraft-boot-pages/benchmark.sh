#!/bin/bash

# verbose output
set -x

source ../common/set-cpus.sh
source ../common/qemu.sh

IMAGES=$(pwd)/images
QEMU_GUEST=qemu-guest

function benchmark {
	taskset -c ${CPU1} ${QEMU_GUEST} \
		-k ${1} -m ${3} -g 1234 -p ${CPU2} \
		-x -P -- -overcommit mem-lock=on

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
		${1} >> ${2}
	kill_qemu
}

suffix=`date '+%d%m%Y%H%M%S'`
mkdir -p rawdata/ results

kill_qemu

RESULTS=results/static.csv
echo "memory	boottime_us" > $RESULTS

LOG=rawdata/1024Mstat-${suffix}.txt
touch $LOG

for j in {1..1}
do
	echo "running batch 1024Mstat, ${j}/10" >> $LOG
	benchmark ${IMAGES}/unikraft+stat.kernel ${LOG} 1024
done

boottime=`cat $LOG | awk -e '$0 ~ /trace_boot_end/ {print $1}' | \
		sed -r '/^\s*$/d' | \
		awk '{ total += $1; count++ } END { print total/(count*1000) }'`
echo "1024	${boottime}" >> $RESULTS

RESULTS=results/dynamic.csv
echo "memory	boottime_us" > $RESULTS

for mem in 32 64 128 256 512 1024 2048 3072;
do
	LOG=rawdata/${mem}Mdyn-${suffix}.txt
	touch $LOG

	for j in {1..1}
	do
		echo "running batch ${mem}Mdyn, ${j}/10" >> $LOG
		if (( $mem > 32 )); then
			benchmark ${IMAGES}/unikraft+dyn.kernel ${LOG} $mem
		else
			benchmark ${IMAGES}/unikraft+dyn32.kernel ${LOG} $mem
		fi
	done

	boottime=`cat $LOG | awk -e '$0 ~ /trace_boot_end/ {print $1}' | \
		sed -r '/^\s*$/d' | \
		awk '{ total += $1; count++ } END { print total/(count*1000) }'`
	echo "${mem}	${boottime}" >> $RESULTS
done
