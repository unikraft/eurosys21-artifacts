#!/bin/bash
set -e

WORKDIR="${1:-.}"
SRC="${1:-.}/src"

##
## benchmark program
##
rm -rvf "${SRC}/build"
rm -rvf "${SRC}/.config"
rm -vf  "${SRC}/*.o"
rm -vf  "${SRC}/fsbench"

##
## intrds and disk images
##
rm -vf "${SRC}/uk-initrd/"*
rm -vf "${SRC}/linux-initrd/"*
rm -vf "${SRC}/fsbench.shfs"
rm -vf "${SRC}/fsbench.cpio"
rm -vf "${SRC}/fsbench.initrd.gz"

##
## directories
##
rm -rvf "${WORKDIR}/cloned"
rm -rvf "${WORKDIR}/eval"

exit $?
