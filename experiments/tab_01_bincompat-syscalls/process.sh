#!/bin/bash
EVAL="${1:-.}/eval"
PARSED="${1:-.}/eval/parsed"
RESULTS="${1:-.}/results"
PROCESS_DS="${1:-.}/do_process_dataset.py"

process()
{
	local TSC_MHZ=$( cat "${EVAL}/tsc_mhz.txt" )

	"${PROCESS_DS}" "${1}" "${TSC_MHZ}"
}

cd "${1:-.}"
mkdir -pv "${RESULTS}"
process "${PARSED}/linux"		>  "$RESULTS/table.txt"
process "${PARSED}/linux-nomitig"	>> "$RESULTS/table.txt"
process "${PARSED}/unikraft"		>> "$RESULTS/table.txt"
