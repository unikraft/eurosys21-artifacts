#!/bin/bash
RESULTS="${1:-.}/eval"
PARSED="${RESULTS}/parsed"

extract_multiple()
{
	cat ${1} | grep -Po "^${2},${3}.*:\K(\d+)"
}

parse_data()
{
	local SERIES=${1}
	local LABEL=$( basename "${SERIES}" )
	local BASE=$( readlink -f $( dirname "${SERIES}" ) )
	local DATA=( $( find ${BASE}/ -regextype sed -regex ".*/${LABEL}\.[0-9]\+" ) )

	echo "Process $LABEL..."
	rm -f ${PARSED}/${LABEL}.shfs-nofile
	rm -f ${PARSED}/${LABEL}.shfs
	rm -f ${PARSED}/${LABEL}.vfs-nofile
	rm -f ${PARSED}/${LABEL}.vfs

	for D in ${DATA[@]}; do
		echo " $D"
		if [ "$2" = "yes" ]; then
			extract_multiple "$D" "SHFS" "<n/a>"  >> ${PARSED}/${LABEL}.shfs-nofile
			extract_multiple "$D" "SHFS" "/"  >> ${PARSED}/${LABEL}.shfs
		fi
		extract_multiple "$D" "VFS"  "<n/a>"  >> ${PARSED}/${LABEL}.vfs-nofile
		extract_multiple "$D" "VFS"  "/"  >> ${PARSED}/${LABEL}.vfs
	done
}

mkdir -pv "${PARSED}"
parse_data "${RESULTS}/linux"
parse_data "${RESULTS}/linux-nomitig"
parse_data "${RESULTS}/unikraft" yes
