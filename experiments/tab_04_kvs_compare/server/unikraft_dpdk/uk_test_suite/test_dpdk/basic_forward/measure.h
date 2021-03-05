#ifndef _UK_DPDK_MEASURE_H
#define _UK_DPDK_MEASURE_H
#include <stdint.h>

#define MEASURE_SIZE    100

struct uk_pkt_measure {
	uint64_t total_txpkts;
	uint64_t total_txpkts_pps[MEASURE_SIZE];
	uint64_t total_txpkts_tries_pps[MEASURE_SIZE];
	uint64_t total_txpkts_tries_ra_pps[MEASURE_SIZE];
	uint64_t total_txpkts_tries_dropped[MEASURE_SIZE];
	uint64_t total_txpkts_alloc_failed[MEASURE_SIZE];
	uint64_t txburst_itr[MEASURE_SIZE];

	uint64_t total_rxpkts;
	uint64_t total_rxpkts_pps[MEASURE_SIZE];
	uint64_t rxpkt_dropped[MEASURE_SIZE];
	uint64_t rxburst_itr[MEASURE_SIZE];

	/* Calculating the computation time */
	uint64_t txpkt_xmit_cycles[MEASURE_SIZE];
	uint64_t txpkt_gen_cycles[MEASURE_SIZE];
	uint64_t txpkt_buf_cycles[MEASURE_SIZE];

	/* Calculating the rx computation time */
	uint64_t rxpkt_recv_cycles[MEASURE_SIZE];
	uint64_t rxpkt_zrecv_cycles[MEASURE_SIZE];
	uint64_t rxpkt_process_cycles[MEASURE_SIZE];
	uint64_t rxpkt_buf_free[MEASURE_SIZE];
	uint64_t latency[MEASURE_SIZE];
};

void dump_stats(int itr);

#endif /* _UK_DPDK_MEASURE_H */
