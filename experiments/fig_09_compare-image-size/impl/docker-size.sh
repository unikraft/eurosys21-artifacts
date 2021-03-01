#!/bin/bash

CONTAINER_LIGHT=hello-world
CONTAINER_FULL=alpine

docker pull $CONTAINER_LIGHT
docker pull $CONTAINER_FULL
LIGHT_SIZE=`docker image inspect $CONTAINER_LIGHT --format='{{.Size}}'`
FULL_SIZE=`docker image inspect $CONTAINER_FULL --format='{{.Size}}'`

echo ${LIGHT_SIZE} > results/docker-hello.csv
echo ${FULL_SIZE} > results/docker-.csv
