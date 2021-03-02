#!/bin/bash

# verbose output
set -x

source ../common/set-cpus.sh
source ../common/qemu.sh

IMAGES=$(pwd)/images
QEMU_GUEST=../tools/qemu-guest

suffix=`date '+%d%m%Y%H%M%S'`
mkdir -p rawdata/ results

LOG=rawdata/solo5-${suffix}.csv
touch $LOG

for j in {1..1}
do
	${IMAGES}/solo5_hvt --mem=2 ${IMAGES}/unikraft+solo5.kernel &> $LOG
done

LOG=rawdata/firecracker-${suffix}.csv
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

	checker_pid=$!

	taskset -c ${CPU1},${CPU2} ${IMAGES}/firecracker \
			--config-file data/firecracker.config \
			--api-sock firecracker.socket

	wait $checker_pid

	rm firecracker.socket
	rm logs.fifo metrics.fifo
done

QEMU_PATH=${IMAGES}/qemu/build/x86_64-softmmu/qemu-system-x86_64
LOG=rawdata/qemumicrovm-${suffix}.csv
touch $LOG
for j in {1..1}
do
	{
		sleep 3
		pkill -9 qemu-system-x86_64
	} &

	checker_pid=$!

	taskset -c ${CPU1},${CPU2} ${QEMU_PATH} \
		-L ${IMAGES}/qemu/pc-bios/ \
		-enable-kvm -nographic -device isa-debug-exit \
		-kernel ${IMAGES}/unikraft+qemu.kernel -m 2M \
		-nodefaults -no-user-config -serial stdio \
		-M microvm,x-option-roms=off,pit=off,pic=off,rtc=on \
		-cpu host,migratable=no,+invtsc \
		-overcommit mem-lock=on &> $LOG

	wait $checker_pid
done

LOG=rawdata/qemu-${suffix}.csv
touch $LOG
for j in {1..1}
do
	{
		sleep 3
		pkill -9 qemu-system-x86_64
	} &

	checker_pid=$!

	taskset -c ${CPU1},${CPU2} ${QEMU_PATH} \
		-L ${IMAGES}/qemu/pc-bios/ \
		-enable-kvm -nographic -device isa-debug-exit \
		-kernel ${IMAGES}/unikraft+qemu.kernel -m 2M \
		-nodefaults -no-user-config -serial stdio \
		-cpu host,migratable=no,+invtsc -no-reboot &> $LOG

	wait $checker_pid
done

LOG=rawdata/qemu1nic-${suffix}.csv
touch $LOG
for j in {1..1}
do
	{
		sleep 3
		pkill -9 qemu-system-x86_64
	} &

	checker_pid=$!

	taskset -c ${CPU1},${CPU2} ${QEMU_PATH} \
		-L ${IMAGES}/qemu/pc-bios/ \
		-enable-kvm -nographic -device isa-debug-exit \
		-kernel ${IMAGES}/unikraft+qemu.kernel -m 2M \
		-nodefaults -no-user-config -serial stdio \
		-cpu host,migratable=no,+invtsc -no-reboot \
		-netdev tap,ifname=qemuif,id=uknetdev,script=no \
		-device virtio-net-pci,netdev=uknetdev,mac=aa:bb:cc:00:01:01 &> $LOG

	wait $checker_pid
done
