#!/bin/bash
CLONED="${1:-./cloned}"
CONFIG="${1:-./src/config.app-elfloader}"

if [ ! -d "$CLONED/elfloader" ]; then
	echo "'elfloader' not found. Did you run 'clone-deps.sh'?"
	exit 1
fi

export UK_ROOT="$( readlink -f "$CLONED" )/unikraft"
export UK_LIBS="$( readlink -f "$CLONED" )/"

set -e
set -x

##
## Copy given Unikernel configuration
##
cp "$CONFIG" "$CLONED/elfloader/.config"

##
## Compile
##
cd "$CLONED/elfloader"
make properclean
make oldconfig
make -j1 fetch
make -j1 prepare
make -j8
