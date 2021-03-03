#!/bin/bash

# verbose output
set -x

source ../common/set-cpus.sh
source ../common/network.sh
source ../common/nginx.sh

LOG=rawdata/docker-nginx.txt
mkdir -p rawdata
touch $LOG

CONTAINER=nginx-bench

function cleanup {
	git checkout ./data/nginx.conf
	# kill all children (evil)
        docker container stop $CONTAINER
        docker rm -f $CONTAINER
	pkill -P $$
}

trap "cleanup" EXIT

sed -i -e "s/daemon off;/#daemon off;/" ./data/nginx.conf

for j in {1..5}
do
	killall -9 nginx

	docker run -v $(pwd)/html:/usr/share/nginx/html:ro --cpus=1 \
		   -v $(pwd)/data:/etc/nginx/ --name=${CONTAINER} \
		   --rm --cpuset-cpus ${CPU2} --privileged --net=host \
		   -d nginx:1.15.6-alpine

	# make sure that the server has properly started
	sleep 2

	# benchmark
	benchmark_nginx_server localhost $LOG

	# stop server
        docker container stop $CONTAINER
        docker rm -f $CONTAINER
	sleep 2
done

git checkout ./data/nginx.conf

RESULTS=results/docker.csv
echo "throughput_reqs" > $RESULTS
parse_nginx_results $LOG $RESULTS
