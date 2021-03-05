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
    local TIMES=${4:-1}
    local APPEND=${5}
    local I=

    for (( I=0; I<${TIMES}; I++ )); do
	taskset -c 2-3 script -c \
		"${TOOLS}/qemu-guest -c 2 -p 4-5 \
		-k ${KERNEL} \
		-i ${INITRD} \
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
run_vm_benchmark    "${RESULTS}/linux"		"linux/vmlinuz-5.11.0-sysnoop+"			\
			"src/syscallbench.initrd.gz"	$TIMES
run_vm_benchmark    "${RESULTS}/linux-nomitig"	"linux/vmlinuz-5.11.0-sysnoop+"			\
			"src/syscallbench.initrd.gz"	$TIMES	"mitigations=off"
run_vm_benchmark    "${RESULTS}/unikraft"	"cloned/elfloader/build/elfloader_kvm-x86_64"	\
			"src/syscallbench"		$TIMES
