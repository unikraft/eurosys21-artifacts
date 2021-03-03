#!/bin/bash

# verbose output
set -x

source ../common/set-cpus.sh

IMAGES=images/

function cleanup {
	# kill all children (evil)
	killall -9 qemu-system-x86
	pkill -9 qemu-system-x86
	pkill -P $$
}

trap "cleanup" EXIT

mkdir -p results

for alloc in "buddy" "tinyalloc" "mimalloc" "tlsf"
do
	LOG=results/${alloc}.csv
	echo "num_queries	speedup" > $LOG

	for queries in 10 100 1000 10000 60000 100000
	do

		for j in {1..10}
		do
			{
				if (( $queries > 10000 )); then
					sleep 8
				else
					sleep 2
				fi
				killall -9 qemu-system-x86
			} &

			script .out -c "taskset -c ${CPU1} qemu-guest \
				-i sqlitebenchmark.cpio \
				-k ${IMAGES}/${queries}/unikraft+${alloc}.kernel \
				-a '' -m 1024 -p ${CPU2}"
			wait

			if ! grep -q "I/O error" .out; then
				res=`cat .out | \
					awk -e '$0 ~ /queries in/ {print $4}'`
				echo "${queries}	${res}" | tee -a $LOG
			fi

			# stop server
			killall -9 qemu-system-x86
			pkill -9 qemu-system-x86
			rm .out
		done
	done
done

# for some reason these experiments mess up the terminal
reset
