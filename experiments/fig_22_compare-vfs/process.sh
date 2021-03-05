#!/bin/bash
PARSED="${1:-.}/eval/parsed"
DATA="${1:-.}/results"
PROCESS_DS="${1:-.}/do_process_dataset.py"

process()
{
	echo "$( basename ${1} )"
	local FNAME=$( basename "${1}" )

	local OS=$(  printf '%s' "$FNAME" | cut -d'.' -f1 )
	local FS=$(   printf '%s' "$FNAME" | cut -d'.' -f2 | cut -d'-' -f1 )
	local TYPE=

	printf '%s' "$FNAME" | cut -d'.' -f2 | grep -q '\(-\)nofile'
	if [ $? -eq 0 ]; then
	    TYPE="no-file"
	else
	    TYPE="file-exists"
	fi

	printf '%s' "$OS" | cut -d'.' -f2 | grep -qe '\(-\)'
	if [ $? -eq 0 ]; then
	    #echo 1+tr
	    #echo $FS
	    OS=$( printf '%s' "${OS}" | tr "[=-=]" "_" )
	    TARGET="${DATA}/${OS}-${FS}.csv"
	else
	    #echo 2conc
	    TARGET="${DATA}/${OS}_${FS}.csv"
	fi

	echo "${TARGET}:${TYPE}"
	echo -n -e "${TYPE}\t" >> "${TARGET}"
	"${PROCESS_DS}" "${1}" >> "${TARGET}"
}

mkdir -pv "${DATA}"
rm -v "${DATA}/"*.csv
process "${PARSED}/linux.vfs"
process "${PARSED}/linux.vfs-nofile"
process "${PARSED}/linux-nomitig.vfs"
process "${PARSED}/linux-nomitig.vfs-nofile"
process "${PARSED}/unikraft.vfs"
process "${PARSED}/unikraft.vfs-nofile"
process "${PARSED}/unikraft.shfs"
process "${PARSED}/unikraft.shfs-nofile"
