#!/bin/bash
CLONED="${1:-.}/cloned"
SRC="${1:-.}/src"
CONFIG="${1:-.}/src/config.app-fsbench"

set -e
set -x

##
## Copy given Unikernel configuration
##
cp "$CONFIG" "$SRC/.config"

##
## Compile Unikernel and linux benchmark
##
export UK_ROOT="$( readlink -f "$CLONED" )/unikraft"
export UK_LIBS="$( readlink -f "$CLONED" )/"
cd "$SRC"

make properclean
make oldconfig
make -j8

make fsbench-clean
make fsbench
