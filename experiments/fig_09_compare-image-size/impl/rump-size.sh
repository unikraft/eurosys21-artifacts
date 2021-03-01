#!/bin/bash

CONTAINER=rump

docker pull hlefeuvre/rump
docker run --privileged --name=$CONTAINER -dt hlefeuvre/rump
HELLO_TOTAL_SIZE=`(docker exec -it $CONTAINER du --block-size=1 \
	/root/hello/hello.img) | tail -n 1 | awk '{ print $1 }'`

docker exec -it $CONTAINER bash -c '. /root/rumprun/obj-amd64-hw/config-PATH.sh && \
		/root/rumprun/./rumprun/bin/x86_64-rumprun-netbsd-strip \
		/root/rumprun-packages/redis/bin/redis-server.img'
REDIS_TOTAL_SIZE=`(docker exec -it $CONTAINER du --block-size=1 \
		/root/rumprun-packages/redis/bin/redis-server.img) | \
		tail -n 1 | awk '{ print $1 }'`

docker exec -it $CONTAINER bash -c '. /root/rumprun/obj-amd64-hw/config-PATH.sh && \
		/root/rumprun/./rumprun/bin/x86_64-rumprun-netbsd-strip \
		/root/rumprun-packages/nginx/bin/nginx.img'
NGINX_TOTAL_SIZE=`(docker exec -it $CONTAINER du --block-size=1 \
		/root/rumprun-packages/nginx/bin/nginx.img) | \
		tail -n 1 | awk '{ print $1 }'`

docker exec -it $CONTAINER bash -c '. /root/rumprun/obj-amd64-hw/config-PATH.sh && \
		/root/rumprun/./rumprun/bin/x86_64-rumprun-netbsd-strip \
		/root/rumprun-packages/pkgs/bin/sqlite3.img'
SQLITE_TOTAL_SIZE=`(docker exec -it $CONTAINER du --block-size=1 \
		/root/rumprun-packages/pkgs/bin/sqlite3.img) | \
		tail -n 1 | awk '{ print $1 }'`

docker container stop $CONTAINER
docker rm -f $CONTAINER

echo ${HELLO_TOTAL_SIZE} > results/rump-hello.csv
echo ${SQLITE_TOTAL_SIZE} > results/rump-sqlite.csv
echo ${NGINX_TOTAL_SIZE} > results/rump-nginx.csv
echo ${REDIS_TOTAL_SIZE} > results/rump-redis.csv
