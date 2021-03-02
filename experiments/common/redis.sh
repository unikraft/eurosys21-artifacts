#!/bin/bash

[[ -z "${REPS}" ]] && REPS=100000
#[[ -z "${CONCURRENT_CONNS}" ]] && CONCURRENT_CONNS=10
[[ -z "${CONCURRENT_CONNS}" ]] && CONCURRENT_CONNS=30
#[[ -z "${PAYLOAD_SIZE}" ]] && PAYLOAD_SIZE=2
[[ -z "${PAYLOAD_SIZE}" ]] && PAYLOAD_SIZE=3
[[ -z "${KEEPALIVE}" ]] && KEEPALIVE=1
#[[ -z "${PIPELINING}" ]] && PIPELINING=1
[[ -z "${PIPELINING}" ]] && PIPELINING=16
[[ -z "${QUERIES}" ]] && QUERIES=get,set

function benchmark_redis_server {
	taskset -c ${CPU3},${CPU4} redis-benchmark --csv -q \
			-n ${REPS} -c ${CONCURRENT_CONNS} \
			-h ${1} -p $2 -d ${PAYLOAD_SIZE} \
			-k ${KEEPALIVE} -t ${QUERIES} \
			-P ${PIPELINING} | tee -a ${LOG}
}

function parse_redis_results {
	getop=`cat $1 | tr ",\"" " " | awk -e '$0 ~ /GET/ {print $2}' | \
		sed -r '/^\s*$/d' | \
		awk '{ total += $1; count++ } END { OFMT="%f"; print total/(count*1000) }'`
	echo "GET	${getop}" >> $2

	setop=`cat $1 | tr ",\"" " " | awk -e '$0 ~ /SET/ {print $2}' | \
		sed -r '/^\s*$/d' | \
		awk '{ total += $1; count++ } END { OFMT="%f"; print total/(count*1000) }'`
	echo "SET	${setop}" >> $2
}
