#!/bin/bash
CLONED="${1:-.}/cloned"
SRC="${1:-.}/src"
CONFIG="${1:-.}/src/config.app-fsbench"

set -e
set -x

##
## Build SHFS tools
##
cd "$CLONED/shfs/tools"

make clean
make
