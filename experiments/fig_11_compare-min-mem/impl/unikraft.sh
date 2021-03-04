#!/bin/bash

# verbose output
set -x

source ../common/set-cpus.sh

function hello_unikraft_with_mem {
    IMAGES=$(pwd)/images/
    CONTAINER=unikraft-tmp

    # remove stale containers
    docker container stop $CONTAINER 2> /dev/null
    docker rm -f $CONTAINER 2> /dev/null

    docker pull hlefeuvre/unikraft-eurosys21:latest
    docker run --rm --privileged --name=$CONTAINER --cpuset-cpus="${CPU1}-${CPU4}" \
			-dt hlefeuvre/unikraft-eurosys21
    docker exec $CONTAINER bash -c \
	"cd app-helloworld && cp configs/stat.conf .config" 2> /dev/null
    docker exec $CONTAINER bash -c \
	"cd app-helloworld && make prepare && make -j" 2> /dev/null

    {
      sleep 3
      docker container stop $CONTAINER 2> /dev/null
      docker rm -f $CONTAINER 2> /dev/null
      sleep 3
    } &

    # record terminal output
    docker exec $CONTAINER script .out -c "/root/qemu-guest -k \
	 app-helloworld/build/app-helloworld_kvm-x86_64 -m $1"

    docker exec $CONTAINER bash -c \
		"grep -iq 'Hello world' .out && echo 'OKBOOT'" > .out

    wait

    grep -q OKBOOT .out
    up=$?
    rm .out

    exit $up
}

function sqlite_unikraft_with_mem {
    IMAGES=$(pwd)/images/
    CONTAINER=unikraft-tmp

    # remove stale containers
    docker container stop $CONTAINER 2> /dev/null
    docker rm -f $CONTAINER 2> /dev/null
    sleep 3

    docker pull hlefeuvre/unikraft-eurosys21:latest
    docker run --rm --privileged --name=$CONTAINER --cpuset-cpus="${CPU1}-${CPU4}" \
			-dt hlefeuvre/unikraft-eurosys21
    docker exec $CONTAINER bash -c \
	"cd app-sqlite && cp configs/tlsf.conf .config" 2> /dev/null
    docker exec $CONTAINER bash -c \
	"cd app-sqlite && make prepare && make -j" 2> /dev/null

    {
      sleep 3
      docker exec $CONTAINER pkill -9 qemu-system-x86
      sleep 3
      docker cp $CONTAINER:/root/workspace/apps/.out /tmp/.out
      docker exec $CONTAINER bash -c \
		"grep -iq 'sqlite>' .out && echo 'OKBOOT'" > .out 2> /dev/null

      docker container stop $CONTAINER 2> /dev/null
      docker rm -f $CONTAINER 2> /dev/null
      sleep 3
    } &

    # record terminal output
    docker exec $CONTAINER script .out -c "/root/qemu-guest -k \
	 app-sqlite/build/app-sqlite_kvm-x86_64 -m $1"

    wait

    grep -q OKBOOT .out
    up=$?
    rm .out

    exit $up
}

function redis_unikraft_with_mem {
    IMAGES=$(pwd)/images/redis
    CONTAINER=unikraft-tmp

    # remove stale containers
    docker container stop $CONTAINER 2> /dev/null
    docker rm -f $CONTAINER 2> /dev/null

    sleep 3

    docker pull hlefeuvre/unikraft-eurosys21:latest
    docker run --rm --privileged --name=$CONTAINER --cpuset-cpus="${CPU1}-${CPU4}" \
			-dt hlefeuvre/unikraft-eurosys21
    docker exec $CONTAINER bash -c \
	"cd app-redis && cp configs/tlsf.conf .config" &> /dev/null
    docker exec $CONTAINER bash -c \
	"cd app-redis && \
	 sed -i 's/CONFIG_LWIP_POOLS=y/# CONFIG_LWIP_POOLS is not set/g' .config" \
	 &> /dev/null
    # note: CONFIG_LWIP_HEAP will auto select
    docker exec $CONTAINER bash -c \
	"cd app-redis && make prepare && make -j" &> /dev/null
    docker exec $CONTAINER /root/setup-networking.sh &> /dev/null

    {
      sleep 6

      docker exec $CONTAINER ping -q -c 1 172.133.1.2
      up=$?

      docker container stop $CONTAINER
      docker rm -f $CONTAINER

      if [ $up -eq 0 ]
      then
	echo "It's alive!"
        exit 0
      else
	echo "Server not alive"
        exit 1
      fi
    } &

    checker_id=$!

    docker exec $CONTAINER bash -c "cd app-redis && \
	/root/qemu-guest -k build/app-redis_kvm-x86_64 \
	 -i redis.cpio -x -m $1 -b br0 \
	 -a \"netdev.ipv4_addr=172.133.1.2 netdev.ipv4_gw_addr=172.133.1.1 \
		netdev.ipv4_subnet_mask=255.255.255.0 -- /redis.conf\""

    wait $checker_id
    exit $?
}

function nginx_unikraft_with_mem {
    IMAGES=$(pwd)/images/nginx
    CONTAINER=unikraft-tmp

    # remove stale containers
    docker container stop $CONTAINER 2> /dev/null
    docker rm -f $CONTAINER 2> /dev/null

    sleep 3

    docker pull hlefeuvre/unikraft-eurosys21:latest
    docker run --rm --privileged --name=$CONTAINER --cpuset-cpus="${CPU1}-${CPU4}" \
			-dt hlefeuvre/unikraft-eurosys21
    docker exec $CONTAINER bash -c \
	"cd app-nginx && cp configs/tlsf.conf .config"
    docker exec $CONTAINER bash -c \
	"cd app-nginx && \
	 sed -i 's/CONFIG_LWIP_POOLS=y/# CONFIG_LWIP_POOLS is not set/g' .config"
    # note: CONFIG_LWIP_HEAP will auto select
    docker exec $CONTAINER bash -c \
	"cd app-nginx && make prepare && make -j"
    docker exec $CONTAINER bash -c \
	"apt install sgabios"
    docker exec $CONTAINER /root/setup-networking.sh

    {
      sleep 6

      docker exec $CONTAINER ping -q -c 1 172.133.1.2
      up=$?

      docker container stop $CONTAINER
      docker rm -f $CONTAINER

      if [ $up -eq 0 ]
      then
	echo "It's alive!"
        exit 0
      else
	echo "Server not alive"
        exit 1
      fi
    } &

    checker_id=$!

    docker exec $CONTAINER bash -c "cd app-nginx && \
	 /root/qemu-guest -k build/app-nginx_kvm-x86_64 \
	 -i nginx.cpio -m $1 -b br0 -x \
	 -a \"netdev.ipv4_addr=172.133.1.2 netdev.ipv4_gw_addr=172.133.1.1 \
		netdev.ipv4_subnet_mask=255.255.255.0 -- \
		-g 'error_log stderr debug;'\""

    wait $checker_id
    exit $?
}
