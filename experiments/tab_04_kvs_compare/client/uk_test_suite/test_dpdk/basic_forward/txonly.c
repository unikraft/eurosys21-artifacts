/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2010-2015 Intel Corporation
 */

#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
#include <rte_eal.h>
#include <rte_ethdev.h>
#include <rte_cycles.h>
#include <rte_lcore.h>
#include <rte_mbuf.h>
#include <testsuite.h>
#include <uk/assert.h>
#include "measure.h"
#include "dev.h"
#include "pktgen.h"

#define PKT_COMPUTATION_BURST		1000
#define PRINT_ITERATION		5

uint8_t  ip_var = 0;
unsigned	arg_nb_bl_pci_addr = 0;
unsigned	arg_ethdev_mask	= 0x01;
unsigned arg_nb_cores_per_ethdev	= 1;
uint16_t arg_nb_queues_per_core	= 8;
uint16_t arg_nb_desc_per_queue = DEF_PKT_BURST;
uint16_t	arg_nb_pkts_per_burst = DEF_PKT_BURST;
 /* a0:36:9f:23:ac:04 */
static uint64_t	arg_stat_timer =  1000 * TIMER_MILLISECOND; /* default period is ~ 1s */
static uint64_t	arg_exp_end_time = 10 * 1000 * TIMER_MILLISECOND;
static uint32_t arg_print_itr = PRINT_ITERATION;
uint32_t	arg_src_ipv4_addr = IPv4(172, 18, 0, 4);
uint32_t	arg_dst_ipv4_addr = IPv4(172, 18, 0, 2);
uint16_t	arg_src_port = 9000;
uint16_t	arg_dst_port = 9001;
uint8_t	arg_ttl	= 10;
uint16_t	arg_pktlen = TXONLY_DEF_PACKET_LEN;
bool arg_statloop = false;
bool arg_receiver = false;
static struct rte_mempool *mbuf_pool;

void dump_stats(int iter)
{
	int i;
	static int j = 1;

	printf("********************* ITERATION: %d *******************\n", j);
	j++;
	/**
	 * Printing packets per second.
	 */
	printf("The count of packet to transmitted/NR of packet sent to virtio per sec/Total iteration\n");
	for ( i = 0; i < iter; i++) {
		printf("%llu/%llu/%llu/%llu/%llu/%llu\n",
		       pkt_stats.total_txpkts_pps[i],
		       pkt_stats.total_txpkts_tries_pps[i],
		       pkt_stats.total_txpkts_tries_ra_pps[i],
		       pkt_stats.total_txpkts_tries_dropped[i],
		       pkt_stats.total_txpkts_alloc_failed[i],
		       pkt_stats.txburst_itr[i]);
	}

	/**
	 * printing total packets
	 */
	printf("The count of packets : %llu\n", pkt_stats.total_txpkts);

	printf("The latency of pkt alloc/pkt gen/ pkt send\n");
	for ( i = 0; i < iter; i++) {
		printf("%llu/%llu/%llu\n",
			pkt_stats.txpkt_buf_cycles[i],
			pkt_stats.txpkt_gen_cycles[i],
			pkt_stats.txpkt_xmit_cycles[i]);
	}
}


/*
 * Initializes a given port using global settings and with the RX buffers
 * coming from the mbuf_pool passed as a parameter.
 */
/*
 * The main function, which does initialization and calls the per-lcore
 * functions.
 */
static int test_tx_only(int argc, char *argv[])
{
	unsigned nb_ports;
	uint16_t portid, first_portid = 0xff;
	int i;

#if 0
	/* Application args process */
	int ret = test_app_args(argc, argv);
#endif

	/* Initialize the Environment Abstraction Layer (EAL). */
	int ret = rte_eal_init(argc, argv);
	if (ret < 0)
		UK_CRASH("Error with EAL initialization\n");

	argc -= ret;
	argv += ret;

	/* Check that there is an even number of ports to send/receive on. */
	nb_ports = rte_eth_dev_count_avail();
	printf("core: %d %d number of ports detected Pool Size: %d\n", rte_lcore_id(), nb_ports, 
		RTE_MBUF_DEFAULT_BUF_SIZE);

	/* Creates a new mempool in memory to hold the mbufs. */
	mbuf_pool = rte_pktmbuf_pool_create("MBUF_POOL", NUM_MBUFS * nb_ports,
					    RTE_MEMPOOL_CACHE_MAX_SIZE, 0,
					    RTE_MBUF_DEFAULT_BUF_SIZE, rte_socket_id());

	if (mbuf_pool == NULL)
		UK_CRASH("Cannot create mbuf pool\n");


	/* Initialize all ports. */
	RTE_ETH_FOREACH_DEV(portid) {
		if (first_portid == 0xff)
			first_portid = portid;
		if (port_init(portid, mbuf_pool) != 0)
			UK_CRASH("Cannot init port %"PRIu16 "\n",
					portid);
	}

	tx_pkt_setup(arg_src_ipv4_addr, arg_dst_ipv4_addr,
		     arg_src_port, arg_dst_port);

	if (rte_lcore_count() > 1)
		printf("\nWARNING: Too many lcores enabled. Only 1 used.\n");

	/* Call lcore_main on the master core only. */
	i = 0;
	int tx_count = 0, total_tx_count = 0;
	uint64_t start_tsc = rte_rdtsc();
	uint64_t curr_tsc, arg_start_tsc = start_tsc, dur;
	int j = 0;
	while (1) {
		while (dur < arg_exp_end_time) {
			tx_count = pkt_burst_transmit(first_portid, mbuf_pool, j);
			pkt_stats.total_txpkts_pps[j] += tx_count;
			pkt_stats.txburst_itr[j]++;
			curr_tsc = rte_rdtsc();
			dur = curr_tsc - arg_start_tsc;

			if (curr_tsc - start_tsc > arg_stat_timer) {

				/* Print stat Computation */
				pkt_stats.total_txpkts += pkt_stats.total_txpkts_pps[j];
				j++;
				if (j == arg_print_itr) {
					dump_stats(j);
					j = 0;
				}
				/* Resetting the tx count */
				pkt_stats.total_txpkts_pps[j] = 0;
				pkt_stats.txburst_itr[j] = 0;
				pkt_stats.total_txpkts_tries_pps[j] = 0;
				pkt_stats.total_txpkts_tries_dropped[j] = 0;
				pkt_stats.total_txpkts_tries_ra_pps[j] = 0;
				pkt_stats.total_txpkts_alloc_failed[j] = 0;
				pkt_stats.txpkt_xmit_cycles[j] = 0;
				pkt_stats.txpkt_gen_cycles[j] = 0;
				pkt_stats.txpkt_buf_cycles[j] = 0;
				/* Restart a new cycle */
				start_tsc = rte_rdtsc();
			}
		}
		arg_start_tsc = rte_rdtsc();
		dur = 0;
	}
	return 0;
}
TESTSUITE_REGISTER_ARGS(test_dpdk, test_tx_only, "start_bfwd",
		"end_bfwd");
