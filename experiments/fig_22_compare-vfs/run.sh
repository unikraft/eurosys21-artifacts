#!/bin/bash
TOOLS="${1:-.}/../../tools"

process_typescript()
{
	cat ${1} | perl -pe 's/\e([^\[\]]|\[.*?[a-zA-Z]|\].*?\a)//g' | col -b > ${2}
}

run_vm_benchmark()
{
    local OUT=${1}
    local KERNEL=${2}
    local INITRD=${3}
    local DISK=${4}
    local TIMES=${5:-1}
    local APPEND=${6}
    local I=

    local QARGS=()

    [ ! -z ${KERNEL} ] && QARGS+=("-k") && QARGS+=("${KERNEL}")
    [ ! -z ${INITRD} ] && QARGS+=("-i") && QARGS+=("${INITRD}")
    [ ! -z ${DISK} ]   && QARGS+=("-d") && QARGS+=("${DISK}")

    for (( I=0; I<${TIMES}; I++ )); do
	taskset -c 2-3 script -c \
		"${TOOLS}/qemu-guest -c 2 -p 4-5 \
		-m 1024 \
		${QARGS[*]} \
		-a 'isolcpus=1 console=ttyS0 panic=-1 ${APPEND}'" ${OUT}.${I}.typescript
	process_typescript ${OUT}.${I}.typescript ${OUT}.${I}
	echo "plain text file is ${OUT}.${I}"
    done
}

RESULTS="${1:-.}/eval"
TIMES=25

cd "${1:-.}"
mkdir -pv "${RESULTS}"
"${TOOLS}/tsc_mhz" > "${RESULTS}/tsc_mhz.txt"
run_vm_benchmark	"${RESULTS}/linux"		"/boot/vmlinuz-$( uname -r )"		"src/fsbench.initrd.gz"	""			$TIMES
run_vm_benchmark	"${RESULTS}/linux-nomitig"	"/boot/vmlinuz-$( uname -r )"		"src/fsbench.initrd.gz"	""			$TIMES "mitigations=off"
run_vm_benchmark	"${RESULTS}/unikraft"		"src/build/fsbench-tlsf_kvm-x86_64"	"src/fsbench.cpio"	"src/fsbench.shfs"	$TIMES
