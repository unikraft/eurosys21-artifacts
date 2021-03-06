#!/bin/bash

WORKDIR=$(realpath $(dirname $0))
SRCDIR=$WORKDIR/src/mirage-solo5-httpreply
IMAGESDIR=$WORKDIR/images
MIRAGE_IMAGE=${MIRAGE_IMAGE:-unikraft/eurosys21-artifacts-mirage:latest}
CONTAINER_NAME=mirage-build-httpreply

cleanup() {
  docker stop $CONTAINER_NAME
}

trap cleanup ERR
trap cleanup EXIT

cp $WORKDIR/html/index.html $SRCDIR/htdocs

# Ensure mirage unikernel directory has correct permissions
chown -Rf 1000:65533 $SRCDIR

# Start the build container
docker run -t -d --rm \
  --name $CONTAINER_NAME \
  -v $SRCDIR:/usr/src/app \
  -w /usr/src/app \
  $MIRAGE_IMAGE

# Configure and build the unikernel
docker exec $CONTAINER_NAME mirage configure -t hvt
docker exec $CONTAINER_NAME make depends
docker exec $CONTAINER_NAME make

mkdir -p $IMAGESDIR
mv $SRCDIR/https.hvt $IMAGESDIR/mirage-solo5-httpreply.hvt
