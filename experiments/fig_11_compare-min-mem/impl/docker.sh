#!/bin/bash

# verbose output
set -x

source ../common/set-cpus.sh
source ../common/network.sh
source ../common/redis.sh

function hello_docker_with_mem {
    IMAGES=$(pwd)/images
    CONTAINER="docker-hello-ram-usage"

    {
      sleep 4
      docker kill $CONTAINER 2> /dev/null
      docker rm -f $CONTAINER 2> /dev/null
    } &

    docker pull hello-world
    docker run -m ${1}M --privileged --name="$CONTAINER" \
			hello-world > .out 2> /dev/null
    wait

    grep -q Hello .out
    up=$?
    rm .out

    exit $up
}

function sqlite_docker_with_mem {
    IMAGES=$(pwd)/images
    CONTAINER="docker-sqlite-ram-usage"

    {
      sleep 5
      docker kill $CONTAINER 2> /dev/null
      docker rm -f $CONTAINER 2> /dev/null
    } &

    docker pull hlefeuvre/alpine-sqlite
    # --tty needed here
    (docker run -m ${1}MB --tty --privileged --name=$CONTAINER \
			hlefeuvre/alpine-sqlite) > .out 2> /dev/null
    wait

    grep -q "sqlite>" .out
    up=$?
    rm .out

    exit $up
}

function redis_docker_with_mem {
    IMAGES=$(pwd)/images
    CONTAINER="redis-tmp-ram"

    docker pull redis:5.0.4-alpine

    {
      sleep 5
      docker kill $CONTAINER
      docker rm -f $CONTAINER
    } &

    checker_id=$!

    (docker run -m ${1}M --tty --privileged --name=$CONTAINER redis:5.0.4-alpine) &> .out
    wait $checker_id

    grep -q "Server initialized" .out
    up=$?
    rm .out

    exit $up
}

function nginx_docker_with_mem {
    IMAGES=$(pwd)/images
    CONTAINER="nginx-tmp-ram"

    docker pull nginx:1.15.6-alpine

    {
      sleep 5
      docker kill $CONTAINER 2> /dev/null
      docker rm -f $CONTAINER 2> /dev/null
    } &

    checker_id=$!

    (docker run -m ${1}M --tty --privileged --name=$CONTAINER \
	nginx:1.15.6-alpine nginx \
	-g 'daemon off;error_log stderr debug;') &> .out
    wait $checker_id

    grep -q "Ready to accept connections" .out
    up=$?
    rm .out

    exit $up
}
