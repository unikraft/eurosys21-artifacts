#!/bin/bash
SRC="${1:-.}/src"
TOOLS="$( readlink -f "${1:-.}/../../tools" )"
SHFSTOOLS="$( readlink -f "${1:-.}/cloned/shfs/tools" )"
set -e
set -x

cd "$SRC"
##
## Create Linux initrd for VFS benchmark
##
cp "fsbench" "linux-initrd/init"
cp "rnd4k" "linux-initrd/7e022976e3522c64794e5a5f2e74fa2cf7e107e99848a6b5f94c9ee9e88a86f1"
"${TOOLS}/mkinitrd" "fsbench.initrd.gz" "linux-initrd/"

##
## Create Unikraft initrd for VFS benchmark
##
cp "rnd4k" "uk-initrd/7e022976e3522c64794e5a5f2e74fa2cf7e107e99848a6b5f94c9ee9e88a86f1"
"${TOOLS}/mkukcpio" "fsbench.cpio" "uk-initrd/"

##
## Create virtual disk for SHFS benchmark
##
dd if=/dev/zero of=fsbench.shfs bs=1M count=16
"${SHFSTOOLS}/shfs_mkfs" -n "fsbench" -s 4096 -f fsbench.shfs

# add file
"${SHFSTOOLS}/shfs_admin" \
	-a "rnd4k" -n "rnd4k" -m "application/octet-stream" \
	-l \
	fsbench.shfs
exit $?
