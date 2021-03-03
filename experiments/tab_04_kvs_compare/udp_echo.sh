#!/bin/bash

file_prefix=cl_tx_rx
pipeline=${1}
interval=${2}
cpufreq=${3}
file_path=${4}

generate_requests()
{
	cat ${1} | grep -A10 "received/transmitted" | grep ^[0-9] | awk -v interval=${3} -F"," 'BEGIN {i=1; printf "Interval, Number of Round Trip Requests\n"; } {if (i%interval == 0)printf "%d,%lu\n", i, $3; i++;}' > ${2}
}

generate_latency()
{
	cat ${1} | grep -A10 "received/transmitted" | grep ^[0-9] | awk -v interval=${3} -v frequency=${4} -F"," 'BEGIN {i=1; printf "Interval, Latency for the round trip in microseconds\n"; } {if (i%interval == 0)printf "%d,%0.04f\n", i, $1/(frequency * $3); i++;}' > ${2}
}

cl_in_file=${file_path}/${file_prefix}_${pipeline}p
[[ -f ${cl_in_file} ]] || { printf "Inputfile ${cl_in_file} missing\n"; exit 1; }
cl_rqst_out_file=${cl_in_file}_rqstpersecond_out
cl_lat_out_file=${cl_in_file}_latency_out


generate_requests ${cl_in_file} ${cl_rqst_out_file} ${interval}
generate_latency ${cl_in_file} ${cl_lat_out_file} ${interval} ${cpufreq}
