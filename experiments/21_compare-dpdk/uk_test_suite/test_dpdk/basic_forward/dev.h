#ifndef _UK_DPDK_DEV_H
#define _UK_DPDK_DEV_H
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
#include <rte_eal.h>
#include <rte_ethdev.h>
#include <rte_cycles.h>
#include <rte_lcore.h>
#include <rte_mbuf.h>
#include <rte_mempool.h>
#ifndef CONFIG_USE_DPDK_PMD
#include <uk/list.h>
#endif /* CONFIG_USE_DPDK_PMD */

#define MAX_PKT_BURST 512
#define DEF_PKT_BURST 128

#define TIMER_MILLISECOND 3200000ULL /* around 1ms at 3.2 Ghz */

#define IPv4(a,b,c,d) ((uint32_t)(((a) & 0xff) << 24) | \
		                 (((b) & 0xff) << 16) | \
                                 (((c) & 0xff) << 8)  | \
				 ((d)  & 0xff))

#ifndef CONFIG_USE_DPDK_PMD
struct uk_ethdev_private {
	struct rte_device dev; /*< allocate a rte_device */
	struct uk_netdev *netdev; /*< Reference to the uk_netdev */
	struct eth_dev_ops dev_ops; /*< Device operations */
	struct rte_eth_stats eth_stats; /*< Device stats */
	struct uk_list_head next;
	struct rte_ring *rx_queue;
	struct rte_ring *tx_queue;
	int tx_burst_fail_count;
	int nb_encap_rx;
	int nb_encap_tx;
	uint16_t max_rx_desc;
	uint16_t max_tx_desc;
};

struct uk_ethdev_queue {
	int port_id;
	int queue_id;                                                                                     struct rte_eth_dev *dev;
}; 
#endif /* CONFIG_USE_DPDK_PMD */


int port_init(uint16_t port, struct rte_mempool *mbuf_pool);
int pkt_burst_transmit(int port_id, struct rte_mempool *mp, int itr);
#ifdef CONFIG_USE_DPDK_PMD
int pkt_burst_receive(int port_id, struct rte_mempool *mpool,
		      struct rte_mbuf **mbufs, int pkt_cnt, uint64_t *total_latency, int itr);
#else
int pkt_burst_receive(int port_id, struct rte_mempool *mpool,
		struct uk_netbuf **mbufs, int pkt_cnt, uint64_t *total_latency, int itr);
#endif
#endif /* _UK_DPDK_DEV_H */
