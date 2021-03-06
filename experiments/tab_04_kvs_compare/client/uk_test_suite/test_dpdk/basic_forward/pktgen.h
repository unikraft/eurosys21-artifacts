#ifndef _UK_DPDK_PKTGEN_H
#define _UK_DPDK_PKTGEN_H
#include <uk/netbuf.h>

#ifdef CONFIG_APPTESTSUITE_DPDK_TXONLY
#define TXONLY_DEF_PACKET_LEN		1500
#else
#define TXONLY_DEF_PACKET_LEN		64
#endif
#define NUM_MBUFS (49152)
//#define NUM_MBUFS (8192)
#define MBUF_CACHE_SIZE 512

#define RTE_BE_TO_CPU_16(be_16_v) \
	    (uint16_t) ((((be_16_v) & 0xFF) << 8) | ((be_16_v) >> 8))
#define RTE_CPU_TO_BE_16(cpu_16_v) \
	    (uint16_t) ((((cpu_16_v) & 0xFF) << 8) | ((cpu_16_v) >> 8))


extern struct uk_pkt_measure pkt_stats;

#ifdef CONFIG_USE_DPDK_PMD
bool pkt_burst_prepare(struct rte_mbuf *pkt, struct rte_mempool *mbp,
		struct rte_ether_hdr *eth_hdr, const uint64_t ol_flags,
		uint64_t timestamp);
bool pkt_burst_prepare_data(struct rte_mbuf *pkt, struct rte_mempool *mbp,
		       struct rte_ether_hdr *eth_hdr, const uint64_t ol_flags,
		       uint64_t timestamp,
		       void *data, uint16_t size);
#else
bool pkt_burst_prepare(struct rte_mbuf *pkt, struct rte_mempool *mbp,
		struct rte_ether_hdr *eth_hdr, const uint64_t ol_flags,
		uint64_t timestamp, struct uk_netbuf **nb);

bool pkt_burst_prepare_data(struct rte_mbuf *pkt, struct rte_mempool *mbp,
		       struct rte_ether_hdr *eth_hdr, const uint64_t ol_flags,
		       uint64_t timestamp, struct uk_netbuf **nb,
		       void *data, uint16_t size);
#endif
void tx_pkt_setup(uint32_t, uint32_t, uint16_t, uint16_t);

#endif /* _UK_DPDK_PKTGEN_H */
