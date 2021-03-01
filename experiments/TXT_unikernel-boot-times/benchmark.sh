#!/bin/bash

# verbose output
set -x

IMAGES=$(pwd)/images/
FIRECRACKER_PATH=$(pwd)/.firecracker
VMMSOCKET=$(pwd)/.firecracker.socket

function cleanup {
  # kill all children (evil)
  pkill -P $$
}

trap "cleanup" EXIT

function firecracker_cleanup {
  sudo pkill -9 firecracker-x86
  sudo pkill -9 firecracker
  sudo pkill -9 firectl
  sudo rm -f $VMMSOCKET
}

function runlupine {
    {
      sleep 5
      firecracker_cleanup
      rm ${IMAGES}/lupine-portwrite.ext2.disp
    } &
    cp ${IMAGES}/lupine-portwrite.ext2 ${IMAGES}/lupine-portwrite.ext2.disp

    touch .out
    firectl --firecracker-binary=$(pwd)/.firecracker \
	--kernel ${IMAGES}/${1}.img \
	--kernel-opts='ro panic=-1 pci=off reboot=k tsc=reliable quiet 8250.nr_uarts=0 ipv6.disable=1 init=/measure-boot-fc' \
	--root-drive=${IMAGES}/lupine-portwrite.ext2.disp \
	--firecracker-log=.out > /dev/null 2> /dev/null

    # get CPU ms
    cat .out | grep Guest-boot-time | rev | awk -e '$0 {print $3}' | \
				rev | awk -e '$0 {print int(1000000*($1))}'
    rm .out
    wait
}

# return boot time for $vm
function runvm {
  vm=$1

  if [ "$vm" = "alpine-vm" ]; then
    cp ${IMAGES}/alpine.raw ${IMAGES}/alpine.raw.disposible
    {
      sleep 8
      firecracker_cleanup
      rm ${IMAGES}/alpine.raw.disposible
    } &
    firectl --kernel=${IMAGES}/hello-vmlinux.bin \
	    --root-drive=${IMAGES}/alpine.raw.disposible \
	    --firecracker-binary=$(pwd)/.firecracker \
	    --firecracker-log=.out > /dev/null 2> /dev/null
    cat .out | grep Guest-boot-time | rev | awk -e '$0 {print $3}' | \
			rev | awk -e '$0 {print int(1000000*($1))}'
    rm .out
    wait
  elif [ "$vm" = "lupine" ]; then
    runlupine $vm
  elif [ "$vm" = "lupine-nokml" ]; then
    runlupine $vm
  elif [ "$vm" = "hermitux" ]; then
    docker pull hlefeuvre/hermitux-boottime > /dev/null 2> /dev/null
    docker run --privileged hlefeuvre/hermitux-boottime | \
		  awk -e '$0 ~ /([0-9]+)ns/ {print $4}' | \
		  cut -c 4- | rev | cut -c 3- | rev
  elif [ "$vm" = "hermitux-light" ]; then
    docker pull hlefeuvre/hermitux-light-boottime > /dev/null 2> /dev/null
    docker run --privileged hlefeuvre/hermitux-light-boottime | \
			awk -e '$0 ~ /([0-9]+)ns/ {print $4}' | \
			cut -c 4- | rev | cut -c 3- | rev
  elif [ "$vm" = "osv-rofs" ]; then
    CONTAINER=osv
    docker pull hlefeuvre/osv > /dev/null 2> /dev/null
    docker run --privileged --name=$CONTAINER \
			-dt hlefeuvre/osv > /dev/null 2> /dev/null
    docker exec -it $CONTAINER ./scripts/firecracker.py -c 1 -m 128M | \
			awk -e '$0 ~ /Booted/ {print int(1000000*($4))}'
    docker container stop $CONTAINER > /dev/null 2> /dev/null
    docker rm -f $CONTAINER > /dev/null 2> /dev/null
    firecracker_cleanup
  elif [ "$vm" = "rump" ]; then
    CONTAINER=rump
    docker pull hlefeuvre/rump-solo5-boottime > /dev/null 2> /dev/null
    docker run --privileged -it hlefeuvre/rump-solo5-boottime |
	awk -e '$0 ~ /guest startup/ {print $4}' | rev | cut -c 3- | rev |
	awk '{print int(1000*($0))}'
  elif [ "$vm" = "docker-min" ]; then
    {
      docker events > .out
    } &
    child=$!
    docker pull hello-world  > /dev/null 2> /dev/null
    taskset -c 8 docker run -m 30MB --privileged \
		-it hello-world > /dev/null 2> /dev/null
    kill $child > /dev/null 2> /dev/null
    LC_NUMERIC="C" grep -P "container (start|die)" .out | tr :+ ' ' | \
		awk '{if(NR>1){print int(1000000000*($3-last))} last=$3}'
    rm .out
  fi
}

# return boot time for $vm, repeated $num times
function benchit  {
  num=$1
  vm=$2

  for i in $( seq 1 ${num} ); do
    runvm $vm >> rawdata/${vm}.txt
  done
}

mkdir -p rawdata

for vm in "alpine-vm" "lupine" "lupine-nokml" "hermitux" \
	  "hermitux-light" "osv-rofs" "rump" "docker-min"
do
  benchit 30 ${vm}
done
