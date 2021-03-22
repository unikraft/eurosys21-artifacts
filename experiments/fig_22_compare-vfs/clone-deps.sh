#!/bin/bash
CLONED="${1:-.}/cloned"

if [ -e "$CLONED" -a ! -d "$CLONED" ]; then
	echo "Target '$CLONED' does exist but is not a directory"
	exit 1
fi

set -e

[ ! -d "$CLONED" ] && mkdir -pv "$CLONED"
cd "$CLONED"

# clone repos if they do not exist yet
[ ! -e "unikraft" ] && git clone --branch skuenzer/eurosys21 'https://github.com/skuenzer/unikraft.git'
[ ! -e "shfs" ]     && git clone --branch skuenzer/eurosys21 'https://github.com/skuenzer/lib-shfs.git' 'shfs'
[ ! -e "tlsf" ]     && git clone --branch skuenzer/eurosys21 'https://github.com/skuenzer/lib-tlsf.git' 'tlsf'
exit 0
