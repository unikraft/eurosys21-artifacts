#!/bin/bash

source ../common/build.sh

# build lupine kernels
build_lupine

# take hermitux app sizes
ROOT_DIR=/root/hermitux/
HELLO_DIR=${ROOT_DIR}/apps/hello-world/
HELLO_LOCATION=${HELLO_DIR}/prog
REDIS_DIR=${ROOT_DIR}/apps/redis-2.0.4/
REDIS_LOCATION=${REDIS_DIR}/redis-server
SQLITE_DIR=${ROOT_DIR}/apps/sqlite/
SQLITE_LOCATION=${SQLITE_DIR}/prog

CONTAINER=hermitux
docker pull olivierpierre/hermitux
docker run --privileged --name=$CONTAINER -dt olivierpierre/hermitux
docker exec -it $CONTAINER make -C $HELLO_DIR
docker exec -it $CONTAINER strip ${HELLO_LOCATION}
HELLO_SIZE=`(docker exec -it $CONTAINER du --block-size=1 ${HELLO_LOCATION}) | \
	tail -n 1 | awk '{ print $1 }'`
docker exec -it $CONTAINER make -C $REDIS_DIR
docker exec -it $CONTAINER strip ${REDIS_LOCATION}
REDIS_SIZE=`(docker exec -it $CONTAINER du --block-size=1 ${REDIS_LOCATION}) | \
	tail -n 1 | awk '{ print $1 }'`
REPLACE="s/\/Desktop//g"
docker exec -it $CONTAINER sed -i -e $REPLACE ${SQLITE_DIR}/Makefile
docker exec -it $CONTAINER make -C $SQLITE_DIR
docker exec -it $CONTAINER strip ${SQLITE_LOCATION}
SQLITE_SIZE=`(docker exec -it $CONTAINER du --block-size=1 ${SQLITE_LOCATION}) | \
	tail -n 1 | awk '{ print $1 }'`
docker container stop $CONTAINER
docker rm -f $CONTAINER
CONTAINER=hermitux

CONTAINER=nginx-tmp
docker pull nginx:1.15.6-alpine
docker run --privileged --name=$CONTAINER -dt nginx:1.15.6-alpine
docker exec -it $CONTAINER apk add binutils
docker exec -it $CONTAINER apk add coreutils
docker exec -it $CONTAINER strip /usr/sbin/nginx
NGINX_SIZE=`(docker exec -it $CONTAINER du --block-size=1 /usr/sbin/nginx) | \
			tail -n 1 | awk '{ print $1 }'`
docker container stop $CONTAINER
docker rm -f $CONTAINER

KERNELBUILD="${LUPINEDIR}/Lupine-Linux/kernelbuild/"

KERNEL_PATH="${KERNELBUILD}/lupine-djw-kml-tiny++hello-world/vmlinux"
strip ${KERNEL_PATH}
KERNEL_SIZE=`du --block-size=1 $KERNEL_PATH | tail -n 1 | awk '{ print $1 }'`
TOTAL_SIZE=$((${KERNEL_SIZE} + ${HELLO_SIZE}))
echo ${TOTAL_SIZE} > results/lupine-hello.csv

KERNEL_PATH="${KERNELBUILD}/lupine-djw-kml-tiny++nginx/vmlinux"
strip ${KERNEL_PATH}
KERNEL_SIZE=`du --block-size=1 $KERNEL_PATH | tail -n 1 | awk '{ print $1 }'`
TOTAL_SIZE=$((${KERNEL_SIZE} + ${NGINX_SIZE}))
echo ${TOTAL_SIZE} > results/lupine-nginx.csv

KERNEL_PATH="${KERNELBUILD}/lupine-djw-kml-tiny++redis/vmlinux"
strip ${KERNEL_PATH}
KERNEL_SIZE=`du --block-size=1 $KERNEL_PATH | tail -n 1 | awk '{ print $1 }'`
TOTAL_SIZE=$((${KERNEL_SIZE} + ${REDIS_SIZE}))
echo ${TOTAL_SIZE} > results/lupine-redis.csv

# reuse redis kernel for sqlite
KERNEL_PATH="${KERNELBUILD}/lupine-djw-kml-tiny++redis/vmlinux"
strip ${KERNEL_PATH}
KERNEL_SIZE=`du --block-size=1 $KERNEL_PATH | tail -n 1 | awk '{ print $1 }'`
TOTAL_SIZE=$((${KERNEL_SIZE} + ${SQLITE_SIZE}))
echo ${TOTAL_SIZE} > results/lupine-sqlite.csv
