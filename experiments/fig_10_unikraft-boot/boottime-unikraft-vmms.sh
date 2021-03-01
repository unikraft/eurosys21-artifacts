#!/bin/bash

# verbose output
set -x

source ../set-cpus.sh
source ../common/qemu.sh

IMAGES=$(pwd)/images
QEMU_GUEST=../tools/qemu-guest
FIRECRACKER_PATH=.firecracker
SOLO5_PATH=.solo5

suffix=`date '+%d%m%Y%H%M%S'`
mkdir -p rawdata/ results

LOG=rawdata/solo5-${suffix}.txt
touch $LOG

for j in {1..1}
do
	${IMAGES}/solo5_hvt --mem=2 ${IMAGES}/unikraft+solo5.kernel &> $LOG
done
