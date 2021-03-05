#!/bin/bash
set -e

WORKDIR="${1:-.}"
SRC="${1:-.}/src"

##
## intrds and disk images
##
rm -vf "${SRC}/linux-initrd/"*
rm -vf "${SRC}/syscallbench.initrd.gz"

##
## benchmark program
##
cd "${SRC}"
make clean

##
## directories
##
rm -rvf "${WORKDIR}/cloned"
rm -rvf "${WORKDIR}/eval"

exit $?
