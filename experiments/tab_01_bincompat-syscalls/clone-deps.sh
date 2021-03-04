#!/bin/bash
CLONED="${1:-./cloned}"

if [ -e "$CLONED" -a ! -d "$CLONED" ]; then
	echo "Target '$CLONED' does exist but is not a directory"
	exit 1
fi

set -e

[ ! -d "$CLONED" ] && mkdir -pv "$CLONED"
cd "$CLONED"

# clone repos if they do not exist yet
[ ! -e "unikraft" ]  && git clone --branch skuenzer/eurosys21 'https://github.com/skuenzer/unikraft.git'
[ ! -e "elfloader" ] && git clone --branch skuenzer/eurosys21 'https://github.com/skuenzer/app-elfloader.git' 'elfloader'
[ ! -e "libelf" ]    && git clone --branch staging 'https://github.com/unikraft/lib-libelf.git' 'libelf'
[ ! -e "zydis" ]     && git clone --branch staging 'https://github.com/unikraft/lib-zydis.git' 'zydis'
[ ! -e "lwip" ]      && git clone --branch staging 'https://github.com/unikraft/lib-lwip.git' 'lwip'
