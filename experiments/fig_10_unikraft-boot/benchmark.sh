#!/bin/bash

# verbose output
set -x

source ../common/set-cpus.sh
source ../common/qemu.sh

IMAGES=$(pwd)/images
QEMU_GUEST=../tools/qemu-guest

suffix=`date '+%d%m%Y%H%M%S'`
mkdir -p rawdata/ results

LOG=rawdata/solo5-${suffix}.txt
touch $LOG

for j in {1..1}
do
	${IMAGES}/solo5_hvt --mem=2 ${IMAGES}/unikraft+solo5.kernel &> $LOG
done

LOG=rawdata/firecracker-${suffix}.txt
touch $LOG
for j in {1..1}
do
	# Create Firecracker FIFOs
	mkfifo logs.fifo
	mkfifo metrics.fifo

	cat logs.fifo > $LOG 2>&1 &

	{
		sleep 3
		pkill -9 firecracker
	} &

	taskset -c ${CPU1},${CPU2} ${IMAGES}/firecracker \
			--config-file data/firecracker.config \
			--api-sock firecracker.socket

	rm firecracker.socket
	rm logs.fifo metrics.fifo
done
