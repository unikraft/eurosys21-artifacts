#!/bin/bash

CONTAINER=osv

docker pull hlefeuvre/osv
docker run --privileged --name=$CONTAINER -dt hlefeuvre/osv
docker exec -it $CONTAINER gcc -v
# rebuild everything just to make sure
docker exec -it $CONTAINER rm -rf /root/osv/build/*
TOTAL_HELLO_SIZE=`(docker exec -it $CONTAINER du --block-size=1 \
	/root/osv/build/release/usr.img) | tail -n 1 | awk '{ print $1 }'`
docker exec -it $CONTAINER ./scripts/build -j4 fs=rofs image=sqlite
TOTAL_SQLITE_SIZE=`(docker exec -it $CONTAINER du --block-size=1 \
	/root/osv/build/release/usr.img) | tail -n 1 | awk '{ print $1 }'`
docker exec -it $CONTAINER ./scripts/build -j4 fs=rofs image=nginx
TOTAL_NGINX_SIZE=`(docker exec -it $CONTAINER du --block-size=1 \
	/root/osv/build/release/usr.img) | tail -n 1 | awk '{ print $1 }'`
docker exec -it $CONTAINER ./scripts/build -j4 fs=rofs image=redis-memonly
TOTAL_REDIS_SIZE=`(docker exec -it $CONTAINER du --block-size=1 \
	/root/osv/build/release/usr.img) | tail -n 1 | awk '{ print $1 }'`
docker container stop $CONTAINER
docker rm -f $CONTAINER

echo ${TOTAL_HELLO_SIZE} > results/osv-hello.csv
echo ${TOTAL_SQLITE_SIZE} > results/osv-sqlite.csv
echo ${TOTAL_NGINX_SIZE} > results/osv-nginx.csv
echo ${TOTAL_REDIS_SIZE} > results/osv-redis.csv
