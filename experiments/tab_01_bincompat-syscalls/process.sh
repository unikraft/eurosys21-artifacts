#!/bin/bash
PARSED="${1:-.}/eval/parsed"
RESULTS="${1:-.}/results"
PROCESS_DS="${1:-.}/do_process_dataset.py"

process()
{
	echo "$( basename ${1} )"
	"${PROCESS_DS}" "${1}"
	echo ""
}

cd "{1:-.}"
mkdir -pv "${RESULTS}"
process "${PARSED}/linux"		>  "$RESULTS/table.txt"
process "${PARSED}/linux-nomitig"	>> "$RESULTS/table.txt"
process "${PARSED}/unikraft"		>> "$RESULTS/table.txt"
