#!/bin/bash

CONTAINER=nginx-tmp
docker pull nginx:alpine
docker run --privileged --name=$CONTAINER -dt nginx:alpine
docker exec -it $CONTAINER apk add binutils
docker exec -it $CONTAINER apk add coreutils
docker exec -it $CONTAINER strip /usr/sbin/nginx
NGINX_SIZE=`(docker exec -it $CONTAINER du --block-size=1 /usr/sbin/nginx) | \
			tail -n 1 | awk '{ print $1 }'`
docker container stop $CONTAINER
docker rm -f $CONTAINER

CONTAINER=redis-tmp
docker pull redis:5.0.6
docker run --privileged --name=$CONTAINER -dt redis:alpine
docker exec -it $CONTAINER apk add binutils
docker exec -it $CONTAINER apk add coreutils
docker exec -it $CONTAINER strip /usr/local/bin/redis-server
REDIS_SIZE=`(docker exec -it $CONTAINER du --block-size=1 \
			/usr/local/bin/redis-server) | \
			tail -n 1 | awk '{ print $1 }'`
docker container stop $CONTAINER
docker rm -f $CONTAINER

gcc data/hello.c -o data/hello
strip data/hello
HELLO_SIZE=`du --block-size=1 data/hello | tail -n 1 | awk '{ print $1 }'`
rm data/hello

echo ${HELLO_SIZE} > results/linuxuser-hello.csv
echo ${NGINX_SIZE} > results/linuxuser-nginx.csv
echo ${REDIS_SIZE} > results/linuxuser-redis.csv
