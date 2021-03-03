#!/bin/bash

[[ -z "${NGINX_BENCH_LENGTH}" ]] && NGINX_BENCH_LENGTH=1m
[[ -z "${NGINX_CONNS}" ]] && NGINX_CONNS=30
[[ -z "${NGINX_WRK_THRDS}" ]] && NGINX_WRK_THRDS=14

function benchmark_nginx_server {
	taskset -c ${CPU3},${CPU4} wrk -t $NGINX_WRK_THRDS \
		-d$NGINX_BENCH_LENGTH -c $NGINX_CONNS \
		http://${1}/index.html | tee -a ${2}
}

function parse_nginx_results {
	op=`cat $1 | awk -e '$0 ~ /Requests\/sec: / {print $2}' | \
		sed -r '/^\s*$/d'`
	echo "${op}" >> $2
}
