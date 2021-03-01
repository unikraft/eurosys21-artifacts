#!/bin/bash

# verbose output
set -x

source ../common/set-cpus.sh
source ../common/network.sh

apt install -y nginx

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
	taskset -c ${CPU3},${CPU4} $WRK -t 14 -d1m -c 30 \
			http://localhost/index.html >> $LOG

	# stop server
	killall -9 nginx
done
