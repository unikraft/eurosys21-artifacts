#!/bin/bash

# verbose output
set -x

source impl/hermitux.sh
source impl/osv.sh
source impl/unikraft.sh
source impl/lupine.sh
source impl/docker.sh
source impl/microvm.sh
source impl/rump.sh
source ../common/network.sh

mkdir -p results

function runvm {
  vm=$1
  app=$2
  mem=$3

  ${app}_${vm}_with_mem $mem &
  pid=$!
  wait $pid
  return $?
}

function dichotomic_search {
  min=$1
  max=$2

  while [[ $min -lt $max ]]; do
    # Compute the mean between min and max, rounded up to the inferior unit
    current=$(( (min + max ) / 2 ))
    echo "trying ${current}..."

    if runvm $3 $4 $current; then
      max=$current
    else
      min=$((current + 1))
    fi
    rm $IMGLOG
  done

  if [ ! -f results/${3}.csv ]; then
    echo "app	minmem" > results/${3}.csv
  fi
  echo "$4	$max" >> results/${3}.csv
}

for vm in "unikraft" "lupine" "rump" "microvm" "docker" "osv" "hermitux"
do
  echo "running $vm / hello"
  dichotomic_search 1 64 ${vm} hello 2> /dev/null
  echo "running $vm / redis"
  dichotomic_search 1 64 ${vm} redis 2> /dev/null
  echo "running $vm / sqlite"
  dichotomic_search 1 64 ${vm} sqlite 2> /dev/null
done

for vm in "unikraft" "lupine" "rump" "microvm" "docker" "osv"
do
  echo "running $vm / nginx"
  dichotomic_search 1 64 ${vm} nginx 2> /dev/null
done
