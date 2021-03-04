#!/bin/bash
SRC="${1:-.}/src"
TOOLS="$( readlink -f "${1:-.}/../../tools" )"
set -e
set -x

cd "$SRC"
make clean syscallbench

# Create intrd for linux containing the benchmark
# Copy the benchmark as init process
cp -f "syscallbench" "linux-initrd/init"
"$TOOLS/mkinitrd" "syscallbench.initrd.gz" "linux-initrd/"
exit $?
