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
#include "pktgen.h"
#include "dev.h"

#define PRINT_ITERATION		10

static uint64_t	arg_stat_timer =  1000 * TIMER_MILLISECOND; /* default period is ~ 1s */
static uint64_t	arg_time_out = 100 * TIMER_MILLISECOND; /* default period is ~ 10 seconds */
/* 2 minutes Duration */
static uint64_t	arg_exp_end_time = 10 * 1000 * TIMER_MILLISECOND;
static uint32_t	arg_src_ipv4_addr = IPv4(172, 18, 0, 4);
static uint32_t	arg_dst_ipv4_addr = IPv4(172, 18, 0, 2);
static uint32_t arg_print_itr = 10;
static uint16_t	arg_src_port = 9000;
static uint16_t	arg_dst_port = 9001;
static uint64_t rx_dropped, tx_dropped;
static uint64_t send_latency, rcv_latency, process_latency, iter;
static int pkt_idx = 0;
static int exp_cnt = 0;
static int pkt_los_rcv = 0;
//static struct rte_ether_addr arg_dst_mac = {{ 0xa0, 0x36, 0x9f, 0x52, 0x29, 0xd0 }};

struct request_header {
	int seq;
	int len;
	struct timespec time;
};

void dump_stats(int itr)
{
	int i;
	printf("Time to generate packets %llu\n",
		pkt_stats.avg_txpkt_gen_cycles); 
	printf("Time to prepare buffers: %llu\n",
		pkt_stats.avg_txpkt_buf_cycles);
	printf("Time to send packets: %llu\n",
		pkt_stats.avg_txpkt_xmit_cycles);

	/**
	 * Printing packets per second.
	 */
	printf("The count of packet to received/transmitted/iteration per sec \n", pkt_stats.rxburst_itr);
	for ( i = 0; i < itr; i++) {
		printf("%llu/%llu/%llu,",
			pkt_stats.total_rxpkts_pps[i], pkt_stats.total_txpkts_pps[i],
			pkt_stats.rxburst_itr[i]);
	}

	/**
	 * printing total packets
	 */
	printf("\nThe count of packets : %llu\n", pkt_stats.total_txpkts);
	printf("The count of packet to received: %llu\n",
		pkt_stats.total_rxpkts);
	printf("\nThe count of rcv dropped packets : %llu\n", rx_dropped);
	printf("\nThe count of snd dropped: %llu\n", tx_dropped);

	/**
	 * printing round trip latency.
	 */
	printf("The receive latency: %llu Prepare latency: %llu Send latency : %llu total_iteration: %llu\n",
		rcv_latency, process_latency, send_latency, iter);

	pkt_stats.avg_txpkt_gen_cycles = 0;
	pkt_stats.avg_txpkt_buf_cycles = 0;
	pkt_stats.avg_txpkt_xmit_cycles = 0;
}

static int echo_packet(struct rte_mbuf *mbuf)
{
	uint16_t offset = 0;
	int rc = -1;
	struct rte_ether_hdr *eth_header = (struct ether_header *)
		rte_pktmbuf_mtod_offset(mbuf, char *, offset);

	struct rte_ether_addr addr;
	struct rte_ipv4_hdr *ip_hdr;
	struct rte_udp_hdr *udp_hdr;
	struct request_header *rsq_hdr;

	if (eth_header->ether_type == 0x8) {
		offset += sizeof(struct rte_ether_hdr);
		ip_hdr = rte_pktmbuf_mtod_offset(mbuf, char *, offset);
		rte_eth_macaddr_get(0, &addr);
		if (ip_hdr->next_proto_id == 0x11) {
			offset += sizeof(struct rte_ipv4_hdr);
			udp_hdr = rte_pktmbuf_mtod_offset(mbuf, char *, offset);

			memcpy(&eth_header->d_addr, &arg_dst_mac, 6);
			memcpy(&eth_header->s_addr, &addr, 6);

			/* Switch IP addresses */
			ip_hdr->src_addr ^= ip_hdr->dst_addr;
			ip_hdr->dst_addr ^= ip_hdr->src_addr;
			ip_hdr->src_addr ^= ip_hdr->dst_addr;

			/* switch UDP PORTS */
			udp_hdr->src_port ^= udp_hdr->dst_port;
			udp_hdr->dst_port ^= udp_hdr->src_port;
			udp_hdr->src_port ^= udp_hdr->dst_port;

			rc = 0;
			/* No checksum requiere, they are 16 bits and
			 * switching them does not influence the checsum
			 * PS: I have also computed the cheksum and it's the same
			 * */
		}
	}

	return rc;
}

static int pkt_burst_rxtx(int port_id, int itr)
{
	uint16_t nb_rx = 0, nb_tx = 0;
	int cnt = 0;
	uint64_t start_tsc, rcv_tsc, snd_tsc, prep_tsc;
	uint16_t offset = sizeof(struct rte_ether_hdr) + sizeof(struct rte_ipv4_hdr) +
			   sizeof(struct rte_udp_hdr);
	struct rte_mbuf *mbufs[DEF_PKT_BURST];
	int i, rc, j;

	start_tsc  = rte_rdtsc();
#define	TX_BURST_SIZE DEF_PKT_BURST
	while (nb_rx < TX_BURST_SIZE)
		nb_rx += rte_eth_rx_burst(port_id, 0, &mbufs[nb_rx], DEF_PKT_BURST - nb_rx);

	rcv_tsc  = rte_rdtsc();

	for (i = 0, j = 0; i < nb_rx; i++) {
		struct rte_mbuf *buf = mbufs[i];
		rte_pktmbuf_reset_headroom(buf);
		pkt_idx++;
		echo_packet(buf);	
		if (rc == -1) {
			rte_pktmbuf_free(buf);
			rx_dropped++;
			continue;
		}
		UK_ASSERT(buf->data_off > 0);
		mbufs[j] = mbufs[i];
		j++;
	}
	prep_tsc = rte_rdtsc();
	if (j > 0)
		nb_tx = rte_eth_tx_burst(port_id, 0, &mbufs[0], j);
	snd_tsc = rte_rdtsc();
	pkt_stats.total_txpkts_pps[itr] += nb_tx;
	pkt_stats.total_rxpkts_pps[itr] += nb_rx;

	while (nb_tx < j) {
		struct rte_mbuf *buf = mbufs[nb_tx];
		rte_pktmbuf_free(buf);
		nb_tx++;
		tx_dropped++;
	}
	send_latency += snd_tsc - prep_tsc;
	rcv_latency += rcv_tsc - start_tsc;
	process_latency += prep_tsc - rcv_tsc;
	iter++;
	pkt_stats.rxburst_itr[itr]++;

	return 1;
}

static int test_closeloop(int argc, char *argv[])
{
	struct rte_mempool *mbuf_pool;
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
	printf("%d number of ports detected Pool Size: %d\n", nb_ports,
		RTE_MBUF_DEFAULT_BUF_SIZE);

	/* Creates a new mempool in memory to hold the mbufs. */
	mbuf_pool = rte_pktmbuf_pool_create("MBUF_POOL", NUM_MBUFS * nb_ports,
		MBUF_CACHE_SIZE, 0, RTE_MBUF_DEFAULT_BUF_SIZE, rte_socket_id());

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

	if (rte_lcore_count() > 1)
		printf("\nWARNING: Too many lcores enabled. Only 1 used.\n");

	/* Call lcore_main on the master core only. */
	i = 0;
	int tx_count = 0, total_tx_count = 0;
	int rx_count = 0, total_rx_count = 0;
	uint64_t start_tsc = rte_rdtsc(); 
	uint64_t curr_tsc;
	uint64_t latency;
	uint64_t total_latency = 0, j = 0;
	uint64_t print_iteration = 1000;
	uint64_t exp_start = start_tsc, exp_curr, dur = 0;
	do {
		while (dur < arg_exp_end_time) {
			tx_count = pkt_burst_rxtx(first_portid, j);
			curr_tsc = rte_rdtsc();
			dur = curr_tsc - exp_start;

			if (curr_tsc - start_tsc > arg_stat_timer) {
				pkt_stats.total_rxpkts += pkt_stats.total_rxpkts_pps[j];
				pkt_stats.total_txpkts += pkt_stats.total_txpkts_pps[j];
				j++;
				if (j == arg_print_itr) {
					/* Print stat Computation */
					dump_stats(j);
					j = 0;
				}
				/* Resetting the tx count */
				pkt_stats.total_txpkts_pps[j] = 0;
				pkt_stats.total_rxpkts_pps[j] = 0;
				pkt_stats.rxburst_itr[j] = 0;

				/* Restart a new cycle */
				start_tsc = rte_rdtsc();
			}
		}
		exp_start = rte_rdtsc();
		dur = 0;
	} while (1);
	return 0;
}
TESTSUITE_REGISTER_ARGS(test_dpdk, test_closeloop, "start_cl","end_cl");
