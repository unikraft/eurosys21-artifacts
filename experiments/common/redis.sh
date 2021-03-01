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

function benchmark_server {
	taskset -c ${CPU3},${CPU4} redis-benchmark --csv -q \
			-n ${REPS} -c ${CONCURRENT_CONNS} \
			-h ${1} -p $2 -d ${PAYLOAD_SIZE} \
			-k ${KEEPALIVE} -t ${QUERIES} \
			-P ${PIPELINING} | tee -a ${LOG}
}
