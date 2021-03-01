#!/bin/bash

filename=`basename $0`

printusage() {
	printf "%s, %s\n" ${filename} tx_filename rx_filename
	printf "tx_filname: Contains traces output from the TXOnly test"
}

process_tx_data_pps() {
	cat $1 | grep -A1 transmitted | grep ^[1-9] |tr , '\n' | grep ^[1-9] | awk -F '/' '{print $1}' | sort -n > $2
	return $?;
}

process_rx_data_pps() {
	cat $1 | grep Rx-pps | awk '{print "%d\n",$2}' | grep ^[1-9] > $2
}

get_stats_data_pps_tx() {
}


if [ $# -ne 2 ]; then
	printusage;
fi

tx_input_file=$1
tx_output_file=${tx_input_file}_ds
rx_input_file=$2
rx_output_file=${rx_output_file}_ds

process_tx_data ${input_file} ${output_file}
[[ $? -ne 0 ]] && { printf "Failed to process tx data\n"; exit 1;}

get_stats ${output_file}
