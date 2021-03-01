#!/bin/bash

ROOT_DIR=/root/hermitux/
KERN_LOCATION=/root/hermitux/hermitux-kernel/prefix/x86_64-hermit/extra/tests/hermitux
HELLO_DIR=/root/hermitux/apps/hello-world/
HELLO_LOCATION=${HELLO_DIR}/prog
REDIS_DIR=${ROOT_DIR}/apps/redis-2.0.4/
REDIS_LOCATION=${REDIS_DIR}/redis-server
SQLITE_DIR=${ROOT_DIR}/apps/sqlite/
SQLITE_LOCATION=${SQLITE_DIR}/prog
CONTAINER=hermitux

docker pull olivierpierre/hermitux
docker run --privileged --name=$CONTAINER -dt olivierpierre/hermitux
docker exec -it $CONTAINER /opt/hermit/bin/x86_64-hermit-gcc -v
docker exec -it $CONTAINER strip ${KERN_LOCATION}
KERNEL_SIZE=`(docker exec -it $CONTAINER du --block-size=1 ${KERN_LOCATION}) | \
	tail -n 1 | awk '{ print $1 }'`
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

TOTAL_SIZE=$((${KERNEL_SIZE} + ${HELLO_SIZE}))
echo ${TOTAL_SIZE} > results/hermituxlight-hello.csv

TOTAL_SIZE=$((${KERNEL_SIZE} + ${REDIS_SIZE}))
echo ${TOTAL_SIZE} > results/hermituxlight-redis.csv

TOTAL_SIZE=$((${KERNEL_SIZE} + ${SQLITE_SIZE}))
echo ${TOTAL_SIZE} > results/hermituxlight-sqlite.csv
