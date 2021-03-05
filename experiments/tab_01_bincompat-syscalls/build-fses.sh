#!/bin/bash
SRC="${1:-.}/src"
TOOLS="$( readlink -f "${1:-.}/../../tools" )"
set -e
set -x

# Create intrd for linux containing the benchmark
# Copy the benchmark as init process
cd "$SRC"
cp -f "syscallbench" "linux-initrd/init"
"$TOOLS/mkinitrd" "syscallbench.initrd.gz" "linux-initrd/"
exit $?
