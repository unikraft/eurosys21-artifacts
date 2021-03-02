#!/bin/bash

# verbose output
set -x

source ../common/set-cpus.sh
source ../common/qemu.sh

IMAGES=$(pwd)/images
QEMU_GUEST=../tools/qemu-guest
REPS=25

suffix=`date '+%d%m%Y%H%M%S'`
mkdir -p rawdata/ results

RESULTS=results/solo5.csv
echo "vmm_us	guest_us" > $RESULTS

LOG=rawdata/solo5-${suffix}.csv
touch $LOG

cp ${IMAGES}/unikraft+solo5.kernel /tmp/unikraft+solo5.kernel
for j in $( eval echo {0..$REPS} )
do
	# note: these two sleeps greatly reduce the variance between
	# results. It is not clear why, but might be due to caching
	# effects, 'warm-up'. Let's assume cold caches as is the case
	# for other boot time measurements (e.g., the loop doesn't
	# execute as fast on qemu because we wait).
	sleep 1
	taskset -c ${CPU1} ${IMAGES}/solo5_hvt --mem=2 \
		/tmp/unikraft+solo5.kernel &> $LOG

	vmm_us=`cat $LOG | awk -e '$0 ~ /solo5 startup/ {print $5}'` &> /dev/null
	guest_vmm=`cat $LOG | awk -e '$0 ~ /guest startup/ {print $4}'` &> /dev/null
	echo "${vmm_us::-2}	${guest_vmm::-2}" >> $RESULTS
	sleep 1
done
rm /tmp/unikraft+solo5.kernel

RESULTS=results/firecracker.csv
echo "vmm_us	guest_us" > $RESULTS

LOG=rawdata/firecracker-${suffix}.csv
touch $LOG
cp ${IMAGES}/unikraft+firecracker.kernel /tmp/unikraft+firecracker.kernel
for j in $( eval echo {0..$REPS} )
do
	# Create Firecracker FIFOs
	mkfifo /tmp/fclogs.fifo
	mkfifo /tmp/fcmetrics.fifo

	cat /tmp/fclogs.fifo > $LOG 2>&1 &

	{
		sleep 4
		pkill -9 firecracker
	} &

	checker_pid=$!

	taskset -c ${CPU1} ${IMAGES}/firecracker \
			--config-file data/firecracker.config \
			--api-sock /tmp/firecracker.socket &> /dev/null

	wait $checker_pid

	vmm_us=`cat $LOG | awk -e '$0 ~ /FC-boot-time/ {print $9}'` &> /dev/null
	guest_vmm=`cat $LOG | awk -e '$0 ~ /Guest-boot-time/ {print $9}'` &> /dev/null
	echo "$vmm_us	$guest_vmm" >> $RESULTS

	rm /tmp/firecracker.socket
	rm /tmp/fclogs.fifo /tmp/fcmetrics.fifo
done
rm /tmp/unikraft+firecracker.kernel

RESULTS=results/qemumicrovm.csv
echo "vmm_us	guest_us" > $RESULTS

QEMU_PATH=${IMAGES}/qemu/build/x86_64-softmmu/qemu-system-x86_64
LOG=rawdata/qemumicrovm-${suffix}.csv
touch $LOG
cp ${IMAGES}/unikraft+qemu.kernel /tmp/unikraft+qemu.kernel
for j in $( eval echo {0..$REPS} )
do
	{
		sleep 3
		pkill -9 qemu-system-x86_64
	} &

	checker_pid=$!

	taskset -c ${CPU1},${CPU2} ${QEMU_PATH} \
		-L ${IMAGES}/qemu/pc-bios/ \
		-enable-kvm -nographic -device isa-debug-exit \
		-kernel /tmp/unikraft+qemu.kernel -m 2M \
		-nodefaults -no-user-config -serial stdio \
		-M microvm,x-option-roms=off,pit=off,pic=off,rtc=on \
		-cpu host,migratable=no,+invtsc \
		-overcommit mem-lock=on &> $LOG

	wait $checker_pid

	vmm_us=`cat $LOG | grep "QEMU  startup" | \
		rev | awk -e '$0 {print $3}' | rev`
	bios_us=`cat $LOG | grep "Pre-platentry" | \
		rev | awk -e '$0 {print $3}' | rev`
	sum=$(( ${vmm_us::-2} + ${bios_us::-2} ))
	guest_vmm=`cat $LOG | grep "Guest startup" | \
		rev | awk -e '$0 {print $3}' | rev`
	echo "${sum}	${guest_vmm::-2}" >> $RESULTS
done

RESULTS=results/qemu.csv
echo "vmm_us	guest_us" > $RESULTS

LOG=rawdata/qemu-${suffix}.csv
touch $LOG
for j in $( eval echo {0..$REPS} )
do
	{
		sleep 3
		pkill -9 qemu-system-x86_64
	} &

	checker_pid=$!

	taskset -c ${CPU1},${CPU2} ${QEMU_PATH} \
		-L ${IMAGES}/qemu/pc-bios/ \
		-enable-kvm -nographic -device isa-debug-exit \
		-kernel /tmp/unikraft+qemu.kernel -m 2M \
		-nodefaults -no-user-config -serial stdio \
		-cpu host,migratable=no,+invtsc -no-reboot &> $LOG

	wait $checker_pid

	vmm_us=`cat $LOG | grep "QEMU  startup" | \
		rev | awk -e '$0 {print $3}' | rev`
	bios_us=`cat $LOG | grep "Pre-platentry" | \
		rev | awk -e '$0 {print $3}' | rev`
	sum=$(( ${vmm_us::-2} + ${bios_us::-2} ))
	guest_vmm=`cat $LOG | grep "Guest startup" | \
		rev | awk -e '$0 {print $3}' | rev`
	echo "${sum}	${guest_vmm::-2}" >> $RESULTS
done

RESULTS=results/qemu1nic.csv
echo "vmm_us	guest_us" > $RESULTS

LOG=rawdata/qemu1nic-${suffix}.csv
touch $LOG
for j in $( eval echo {0..$REPS} )
do
	{
		sleep 3
		pkill -9 qemu-system-x86_64
	} &

	checker_pid=$!

	taskset -c ${CPU1},${CPU2} ${QEMU_PATH} \
		-L ${IMAGES}/qemu/pc-bios/ \
		-enable-kvm -nographic -device isa-debug-exit \
		-kernel /tmp/unikraft+qemu.kernel -m 2M \
		-nodefaults -no-user-config -serial stdio \
		-cpu host,migratable=no,+invtsc -no-reboot \
		-netdev tap,ifname=qemuif,id=uknetdev,script=no \
		-device virtio-net-pci,netdev=uknetdev,mac=aa:bb:cc:00:01:01 &> $LOG

	wait $checker_pid

	vmm_us=`cat $LOG | grep "QEMU  startup" | \
		rev | awk -e '$0 {print $3}' | rev`
	bios_us=`cat $LOG | grep "Pre-platentry" | \
		rev | awk -e '$0 {print $3}' | rev`
	sum=$(( ${vmm_us::-2} + ${bios_us::-2} ))
	guest_vmm=`cat $LOG | grep "Guest startup" | \
		rev | awk -e '$0 {print $3}' | rev`
	echo "${sum}	${guest_vmm::-2}" >> $RESULTS
done
rm /tmp/unikraft+qemu.kernel

# qemu messes up the terminal, reset it
reset
