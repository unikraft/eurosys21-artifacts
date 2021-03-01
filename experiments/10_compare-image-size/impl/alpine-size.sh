#!/bin/bash

URL=http://dl-cdn.alpinelinux.org/alpine/v3.12/releases/x86_64/alpine-virt-3.12.1-x86_64.iso

wget $URL
TOTAL_SIZE=`(du --block-size=1 ./alpine-virt-3.12.1-x86_64.iso) | \
	tail -n 1 | awk '{ print $1 }'`
rm ./alpine-virt-3.12.1-x86_64.iso

echo ${TOTAL_SIZE} > results/alpine-.csv
