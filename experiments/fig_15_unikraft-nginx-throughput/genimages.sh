#!/bin/bash

set -x

IMAGES=$(pwd)/images

source ../common/build.sh

rm -rf $IMAGES
mkdir -p $IMAGES

# ========================================================================
# Generate Unikraft images
# ========================================================================

function build_for {
	unikraft_eurosys21_build $1 $2 $IMAGES
}

build_for nginx tinyalloc
build_for nginx mimalloc
build_for nginx tlsf
build_for nginx buddy
