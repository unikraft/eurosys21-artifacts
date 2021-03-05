#!/bin/bash
RESULTS="${1:-.}/eval"
PARSED="${RESULTS}/parsed"

extract_data()
{
	cat ${1} | grep -Po "^${2}\t\K(\d+)"
}

parse_data()
{
	local SERIES=${1}
	local LABEL=$( basename "${SERIES}" )
	local BASE=$( readlink -f $( dirname "${SERIES}" ) )
	local DATA=( $( find ${BASE}/ -regextype sed -regex ".*/${LABEL}\.[0-9]\+" ) )

	echo "Process $LABEL..."
	rm -f ${PARSED}/${LABEL}.tsc
	rm -f ${PARSED}/${LABEL}.fcall
	rm -f ${PARSED}/${LABEL}.scall

	for D in ${DATA[@]}; do
		echo " $D"
		extract_data "$D" "tsc"   >> ${PARSED}/${LABEL}.tsc
		extract_data "$D" "fcall" >> ${PARSED}/${LABEL}.fcall
		extract_data "$D" "scall" >> ${PARSED}/${LABEL}.scall
	done
}

mkdir -pv "${PARSED}"
parse_data "${RESULTS}/linux"
parse_data "${RESULTS}/linux-nomitig"
parse_data "${RESULTS}/unikraft"
