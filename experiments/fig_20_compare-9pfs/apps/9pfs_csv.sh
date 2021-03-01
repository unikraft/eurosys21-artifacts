#!/bin/bash
RAW=$1
SIZE=1024
TIME=1000
block_size=( $(cat $RAW|grep "block size"|awk '{print $7}') )
latency=( $(cat $RAW|grep "avg operation latency"|awk '{print $4}'))
echo "block_size_kb,avg_latency_microseconds"
for element in $(seq 0 $((${#block_size[@]} - 1)))
do
      b="${block_size[$element]}"
      b_mb=$(echo "$b $SIZE" | awk '{print $1 / $2}')
      t="${latency[$element]}"
      t_us=$(echo "$t $TIME" | awk '{print $1 / $2}')
      echo "$b_mb,$t_us"
done