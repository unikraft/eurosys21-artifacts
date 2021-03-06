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
#include <uk/config.h>
#include <uk/netdev.h>

#define PRINT_ITERATION		10
#define RX_NUM_BUF 16384
#define TX_NUM_BUF 16384

static uint64_t	arg_stat_timer =  1000 * TIMER_MILLISECOND; /* default period is ~ 1s */
static uint64_t	arg_time_out = 100 * TIMER_MILLISECOND; /* default period is ~ 10 seconds */
/* 2 minutes Duration */
static uint64_t	arg_exp_end_time = 10 * 1000 * TIMER_MILLISECOND;
static uint32_t	arg_src_ipv4_addr = IPv4(172, 17, 0, 4);
static uint32_t	arg_dst_ipv4_addr = IPv4(172, 17, 0, 114);
static uint32_t arg_print_itr = 1;
static uint16_t	arg_src_port = 9000;
static uint16_t	arg_dst_port = 9001;
static int  burst_cnt = DEF_PKT_BURST;
static uint64_t tsc;

void dump_stats(int iter)
{
	int i;
	static int j = 1;
	printf("******iteration: %d*********************\n", j);
	j++;
	/**
	 * Printing packets latency per second.
	 */
	printf("The latency, iteration, count of packet to received/transmitted per sec\n");
	for ( i = 0; i < iter; i++) {
		printf("%llu,%llu,%llu,%llu\n", pkt_stats.latency[i],
			pkt_stats.rxburst_itr[i], pkt_stats.total_rxpkts_pps[i],
			pkt_stats.rxpkt_dropped[i]);
	}

	/**
	 * printing total packets
	 */
	printf("The count of packet to received: %llu\n",
		pkt_stats.total_rxpkts);

#if 0
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
#if 0
	pkt_stats.avg_txpkt_gen_cycles = 0;
	pkt_stats.avg_txpkt_buf_cycles = 0;
	pkt_stats.avg_txpkt_xmit_cycles = 0;
#endif

	printf("The latency of pkt alloc/pkt gen/ pkt send\n");
	for ( i = 0; i < iter; i++) {
		printf("%llu/%llu/%llu\n",
			pkt_stats.txpkt_buf_cycles[i],
			pkt_stats.txpkt_gen_cycles[i],
			pkt_stats.txpkt_xmit_cycles[i]);
	}
#endif

	printf("The latency of pkt recv/0 pkt recv/pkt process/ pkt free\n");
	for ( i = 0; i < iter; i++) {
		printf("%llu/%llu/%llu/%llu\n",
			pkt_stats.rxpkt_recv_cycles[i],
			pkt_stats.rxpkt_zrecv_cycles[i],
			pkt_stats.rxpkt_process_cycles[i],
			pkt_stats.rxpkt_buf_free[i]);
	}
}

int process_arpreply(struct rte_mbuf *mbuf, int port)
{
    struct rte_mbuf *pkts_burst[MAX_PKT_BURST];
    struct rte_mbuf *pkt = mbuf;
    struct rte_ether_hdr *eth_h;
    struct rte_vlan_hdr *vlan_h;
    struct rte_arp_hdr  *arp_h;
    struct rte_ipv4_hdr *ip_h;
    struct rte_icmp_hdr *icmp_h = NULL;
    struct rte_udp_hdr *udp_h = NULL;
    struct rte_ether_addr eth_addr;
    uint32_t retry;
    uint32_t ip_addr;
    uint16_t nb_rx;
    uint16_t nb_tx;
    uint16_t nb_replies;
    uint16_t eth_type;
    uint16_t vlan_id;
    uint16_t arp_op;
    uint16_t arp_pro;
    uint16_t udp_port;
    uint32_t cksum;
	int l2_len;
	int verbose_level = 1;
 	struct rte_ether_addr arg_dst_mac = {{ 0xa0, 0x36, 0x9f, 0x52, 0x29, 0xD0 }};
	struct rte_ether_addr addr;
		rte_eth_macaddr_get(port, &addr);
		eth_h = rte_pktmbuf_mtod(pkt, struct rte_ether_hdr *);
		eth_type = RTE_BE_TO_CPU_16(eth_h->ether_type);
		  l2_len = sizeof(struct rte_ether_hdr);

        if (eth_type == RTE_ETHER_TYPE_ARP) {
            arp_h = (struct rte_arp_hdr *) ((char *)eth_h + l2_len);
            arp_op = RTE_BE_TO_CPU_16(arp_h->arp_opcode);
            arp_pro = RTE_BE_TO_CPU_16(arp_h->arp_protocol);
	#if 0
            if (verbose_level > 0) {
                printf("  ARP:  hrd=%d proto=0x%04x hln=%d "
                       "pln=%d op=%u (%s)\n",
                       RTE_BE_TO_CPU_16(arp_h->arp_hardware),
                       arp_pro, arp_h->arp_hlen,
                       arp_h->arp_plen, arp_op,
                       arp_op_name(arp_op));
            }
#endif
            if ((RTE_BE_TO_CPU_16(arp_h->arp_hardware) !=
                 RTE_ARP_HRD_ETHER) ||
                (arp_pro != RTE_ETHER_TYPE_IPV4) ||
                (arp_h->arp_hlen != 6) ||
                (arp_h->arp_plen != 4)
                ) {
                rte_pktmbuf_free(pkt);
                if (verbose_level > 0)
                    printf("\n");
				return -1;
            }
            if (verbose_level > 0) {
                rte_ether_addr_copy(&arp_h->arp_data.arp_sha,
                        &eth_addr);
                //ether_addr_dump("        sha=", &eth_addr);
                ip_addr = arp_h->arp_data.arp_sip;
                //ipv4_addr_dump(" sip=", ip_addr);
                printf("\n");
                rte_ether_addr_copy(&arp_h->arp_data.arp_tha,
                        &eth_addr);
                //ether_addr_dump("        tha=", &eth_addr);
                ip_addr = arp_h->arp_data.arp_tip;
                //ipv4_addr_dump(" tip=", ip_addr);
                printf("\n");
            }
            if (arp_op != RTE_ARP_OP_REQUEST) {
                rte_pktmbuf_free(pkt);
				return -1;
            }
            /*
             * Build ARP reply.
             */
            /* Use source MAC address as destination MAC address. */
            rte_ether_addr_copy(&eth_h->s_addr, &eth_h->d_addr);
            /* Set source MAC address with MAC address of TX port */
            rte_ether_addr_copy(&arg_dst_mac,
                    &eth_h->s_addr);
            arp_h->arp_opcode = rte_cpu_to_be_16(RTE_ARP_OP_REPLY);
            rte_ether_addr_copy(&arp_h->arp_data.arp_tha,
                    &eth_addr);
            rte_ether_addr_copy(&arp_h->arp_data.arp_sha,
                    &arp_h->arp_data.arp_tha);
            rte_ether_addr_copy(&eth_h->s_addr,
                    &arp_h->arp_data.arp_sha);
            /* Swap IP addresses in ARP payload */
            ip_addr = arp_h->arp_data.arp_sip;
            arp_h->arp_data.arp_sip = arp_h->arp_data.arp_tip;
            arp_h->arp_data.arp_tip = ip_addr;
			printf("ARP reply generated\n");
			return 0;
        }  
		return -1;
}

#ifdef CONFIG_USE_DPDK_PMD
uint64_t process_packet(struct rte_mbuf **mbufs, int cnt, int idx)
{
	uint64_t start_tsc, end_tsc;
	uint64_t latency;
	uint16_t offset = sizeof(struct rte_ether_hdr) + sizeof(struct rte_ipv4_hdr) +
			   sizeof(struct rte_udp_hdr);
	int i;

	start_tsc = rte_rdtsc();
	for (i = 0; i < cnt; i++) {
		struct rte_mbuf *buf = mbufs[i];
		uint64_t *sent_tsc = rte_pktmbuf_mtod_offset(buf,
				char *, offset);
		UK_ASSERT(sent_tsc && buf->buf_addr);
		latency += start_tsc - *sent_tsc;
	}
	end_tsc = rte_rdtsc();
	pkt_stats[idx].rxpkt_process_cycles += (end_tsc - start_tsc);
	return latency;
}
#else
uint64_t process_packet(struct uk_netbuf **nb, int cnt, int idx)
{
	uint64_t start_tsc, end_tsc;
	uint64_t latency = 0;
	uint16_t offset = sizeof(struct rte_ether_hdr) + sizeof(struct rte_ipv4_hdr) +
			   sizeof(struct rte_udp_hdr);
	int i;

	start_tsc = rte_rdtsc();
	for (i = 0; i < cnt; i++) {
		UK_ASSERT(nb[i]);
		struct rte_mbuf *mbuf = nb[i]->priv;
		mbuf->pkt_len = nb[i]->len;
		mbuf->data_len = nb[i]->len;
		uint64_t *sent_tsc = rte_pktmbuf_mtod_offset(mbuf,
				char *, offset);
		UK_ASSERT(sent_tsc && mbuf->buf_addr);
		latency += start_tsc - *sent_tsc;
	}
	end_tsc = rte_rdtsc();
	pkt_stats.rxpkt_process_cycles[idx] += (end_tsc - start_tsc);

	return latency;
}
#endif

int pkt_burst_receive_arp(int port_id, struct rte_mempool *mpool,
			  int idx)
{
	struct uk_netbuf *bufs[DEF_PKT_BURST];
	uint16_t pkt = 0;
	uint64_t latency;
	      struct rte_eth_dev *vrtl_eth_dev;                                                                                                                                                                                                                                         
      struct uk_ethdev_private *dev_private;
      vrtl_eth_dev = &rte_eth_devices[port_id];
      UK_ASSERT(vrtl_eth_dev);
      dev_private = vrtl_eth_dev->data->dev_private;
      UK_ASSERT(dev_private);

	while (pkt == 0) {
		pkt = pkt_burst_receive(port_id, mpool, &bufs[0], DEF_PKT_BURST,
			 &latency, idx);
		if (pkt == 0)
			continue;
		int rc = process_arpreply(bufs[0]->priv, port_id);
		if (rc < 0) {
			printf("Failed to process the arp requeust");
			pkt = 0;
		} else {	
			uint16_t nb_tx = 1;
			rc = uk_netdev_tx_burst(dev_private->netdev,
                              0, &bufs[0], &nb_tx);
			UK_ASSERT(rc >= 0);
			printf("Send arp reply\n");
		}
	}
}

int pkt_burst_receive_cnt(int port_id, struct rte_mempool *mpool,
			  int idx)
{
	int pkt = 0;
	int i;

	uint64_t start_tsc, end_tsc;
	uint64_t curr_tsc;
#ifdef CONFIG_USE_DPDK_PMD
	struct rte_mbuf *bufs[DEF_PKT_BURST];
#else
	struct uk_netbuf *bufs[DEF_PKT_BURST];
#endif
	uint64_t latency = 0;

	start_tsc = rte_rdtsc();
	while (pkt == 0) {
		pkt = pkt_burst_receive(port_id, mpool, &bufs[0], DEF_PKT_BURST,
			 &latency, idx);
		if (pkt > 0)
			break;

		curr_tsc = rte_rdtsc();
		if (pkt == 0 && (curr_tsc - start_tsc) > (TIMER_MILLISECOND * 50))
			return -1;
	}

	pkt_stats.latency[idx] += process_packet(bufs, pkt, idx);

	start_tsc = rte_rdtsc();
#ifdef CONFIG_USE_DPDK_PMD
	for (i = 0; i < pkt; i++) {
		rte_pktmbuf_free(bufs[i]);
	}
#else
	for (i = 0; i < pkt; i++) {
		UK_ASSERT(bufs[i]);
		UK_ASSERT(bufs[i]->priv);
		rte_pktmbuf_free(bufs[i]->priv);
	}
#endif
	end_tsc = rte_rdtsc();
	pkt_stats.rxpkt_buf_free[idx] += (end_tsc - start_tsc);
	latency = 0;
	pkt_stats.rxburst_itr[idx]++;

	return pkt;
}

void pkt_burst_data_get(void **data, uint16_t *size)
{
	tsc =  rte_rdtsc();
	*data = &tsc;
	*size = sizeof(tsc);
}

int pkt_burst_transmit_cnt(int port_id, struct rte_mempool *mp, int cnt, int itr)
{
	int pkts = 0;
	int tx_cnt = 0;

	void *data;
	uint16_t size;
	int burst = DEF_PKT_BURST;

	while (pkts < cnt) {
		pkt_burst_data_get(&data, &size);
		if (cnt - pkts < burst)
			burst = cnt - pkts;
		tx_cnt  = pkt_burst_transmit_data(port_id, mp, itr, burst, data, size);
		pkts += tx_cnt;
	}
	return pkts;
}


static int test_tx_rx(int argc, char *argv[])
{
	struct rte_mempool *mbuf_pool;
	unsigned nb_ports;
	uint16_t portid, first_portid = 0xff;
	int i;
	int pkt_in_flight = CONFIG_INPIPELINE;

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
					    RTE_MEMPOOL_CACHE_MAX_SIZE, 0,
					    RTE_MBUF_DEFAULT_BUF_SIZE, rte_socket_id());


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
	int rx_count = 0, total_rx_count = 0;
	uint64_t start_tsc = rte_rdtsc(); 
	uint64_t curr_tsc;
	uint64_t latency;
	uint64_t j = 0;
	int cnt = CONFIG_INPIPELINE;
	uint64_t exp_start = start_tsc, exp_curr, dur = 0;
#if 0
	pkt_burst_transmit_cnt(first_portid, mbuf_pool, 1, 0);
	int rc = pkt_burst_receive_arp(first_portid, mbuf_pool, 0);
#endif
	pkt_stats.rxpkt_dropped[0] = 0;
	do {
		while (dur < arg_exp_end_time) {
			if (cnt > 0) {
				tx_count = pkt_burst_transmit_cnt(first_portid, mbuf_pool, cnt, j);
				pkt_stats.total_txpkts_pps[j] += tx_count;
				pkt_stats.txburst_itr[j]++;
				cnt -= tx_count;
			}
			int rc = pkt_burst_receive_cnt(first_portid, mbuf_pool, j);
			if (unlikely(rc == -1)) {
				/**
				 * Resetting the pipeline because packet loss.
				 */
				rx_count = 0;
				pkt_stats.rxpkt_dropped[j] += CONFIG_INPIPELINE;
				cnt += CONFIG_INPIPELINE;
			} else {
				rx_count = rc;
				pkt_stats.total_rxpkts_pps[j] += rc;
				cnt += rx_count;
			}
			/* Send the remaining packet */
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
				pkt_stats.latency[j] = 0;
				pkt_stats.rxburst_itr[j] = 0;
				pkt_stats.txburst_itr[j] = 0;
				pkt_stats.total_txpkts_tries_pps[j] = 0;
				pkt_stats.total_txpkts_tries_dropped[j] = 0;
				pkt_stats.total_txpkts_tries_ra_pps[j] = 0;
				pkt_stats.total_txpkts_alloc_failed[j] = 0;
				pkt_stats.txpkt_xmit_cycles[j] = 0;
				pkt_stats.txpkt_gen_cycles[j] = 0;
				pkt_stats.txpkt_buf_cycles[j] = 0;
				pkt_stats.rxpkt_recv_cycles[j] = 0;
				pkt_stats.rxpkt_zrecv_cycles[j] = 0;
				pkt_stats.rxpkt_process_cycles[j] = 0;
				pkt_stats.rxpkt_buf_free[j] = 0;
				pkt_stats.rxpkt_dropped[j] = 0;

				/* Restart a new cycle */
				start_tsc = rte_rdtsc();
			}
		}
		exp_start = rte_rdtsc();
		dur = 0;
	} while (1);
	return 0;
}
TESTSUITE_REGISTER_ARGS(test_dpdk, test_tx_rx, "start_rxonly","end_rxonly");
