#!/bin/bash

# verbose output
set -x

source ../common/set-cpus.sh
source ../common/network.sh
source ../common/nginx.sh

apt install -y nginx
update-rc.d -f nginx disable
# this might be a bug in the Debian package...
mkdir -p /var/log/nginx/

LOG=rawdata/native-nginx.txt
mkdir -p rawdata
touch $LOG

function cleanup {
	# kill all children (evil)
	killall -9 nginx
	pkill -P $$
}

trap "cleanup" EXIT

for j in {1..5}
do
	taskset -c ${CPU2} nginx -c $(pwd)/data/nginx.conf &

	# make sure that the server has properly started
	sleep 2

	# benchmark
	benchmark_nginx_server localhost $LOG

	# stop server
	killall -9 nginx
done

RESULTS=results/native.csv
echo "throughput_reqs" > $RESULTS
parse_nginx_results $LOG $RESULTS
