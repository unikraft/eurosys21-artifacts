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
#ifndef CONFIG_USE_DPDK_PMD
#include <uk/netbuf.h>
#include <uk/netdev.h>
#endif /* CONFIG_USE_DPDK_PMD */
#include "pktgen.h"
#include "measure.h"
#include "dev.h"

#define RX_RING_SIZE 1024
#define TX_RING_SIZE 1024

static const struct rte_eth_conf port_conf_default = {
	.rxmode = {
		.max_rx_pkt_len = RTE_ETHER_MAX_LEN,
	},
};


static struct rte_ether_addr arg_dst_mac = {{ 0x68,0x05,0xca,0x0c,0x47,0x51}};
//static struct rte_ether_addr arg_dst_mac = {{ 0xa0, 0x36, 0x9f, 0x23, 0xac, 0x06 }};
static uint16_t nb_pkt_per_burst = DEF_PKT_BURST; /**< Number of packets per burst. */

int port_init(uint16_t port, struct rte_mempool *mbuf_pool)
{
	struct rte_eth_conf port_conf = port_conf_default;
	const uint16_t rx_rings = 1, tx_rings = 1;
	uint16_t nb_rxd = RX_RING_SIZE;
	uint16_t nb_txd = TX_RING_SIZE;
	int retval;
	uint16_t q;
	struct rte_eth_dev_info dev_info;
	struct rte_eth_txconf txconf;

	if (!rte_eth_dev_is_valid_port(port))
		return -1;


	printf("%s: Memory pool: %d\n", __func__, mbuf_pool);
	rte_eth_dev_info_get(port, &dev_info);
	if (dev_info.tx_offload_capa & DEV_TX_OFFLOAD_MBUF_FAST_FREE)
		port_conf.txmode.offloads |=
			DEV_TX_OFFLOAD_MBUF_FAST_FREE;


	/* Configure the Ethernet device. */
	retval = rte_eth_dev_configure(port, rx_rings, tx_rings, &port_conf);
	if (retval != 0)
		return retval;

	uk_netdev_hwaddr_get(uk_netdev_get(0));
	retval = rte_eth_dev_adjust_nb_rx_tx_desc(port, &nb_rxd, &nb_txd);
	if (retval != 0)
		return retval;

	/* Allocate and set up 1 RX queue per Ethernet port. */
	for (q = 0; q < rx_rings; q++) {
		retval = rte_eth_rx_queue_setup(port, q, nb_rxd,
				rte_eth_dev_socket_id(port), NULL, mbuf_pool);
		if (retval < 0)
			return retval;
	}

	txconf = dev_info.default_txconf;
	txconf.offloads = port_conf.txmode.offloads;
	/* Allocate and set up 1 TX queue per Ethernet port. */
	for (q = 0; q < tx_rings; q++) {
		retval = rte_eth_tx_queue_setup(port, q, nb_txd,
				rte_eth_dev_socket_id(port), &txconf);
		if (retval < 0)
			return retval;
	}

	/* Start the Ethernet port. */
	retval = rte_eth_dev_start(port);
	if (retval < 0)
		return retval;

	/* Display the port MAC address. */
	struct rte_ether_addr addr;
	rte_eth_macaddr_get(port, &addr);
	printf("Port %u MAC: %02" PRIx8 " %02" PRIx8 " %02" PRIx8
			   " %02" PRIx8 " %02" PRIx8 " %02" PRIx8 "\n",
			port,
			addr.addr_bytes[0], addr.addr_bytes[1],
			addr.addr_bytes[2], addr.addr_bytes[3],
			addr.addr_bytes[4], addr.addr_bytes[5]);

	/* Enable RX in promiscuous mode for the Ethernet device. */
	rte_eth_promiscuous_enable(port);

	return 0;
}

int pkt_burst_transmit_data(int port_id, struct rte_mempool *mp, int itr,
			    int cnt, void *data, uint16_t size)
{
	struct rte_mbuf *pkts_burst[MAX_PKT_BURST];
	struct uk_netbuf *uk_pkts_burst[MAX_PKT_BURST];
	struct rte_port *txp;
	struct rte_mbuf *pkt;
	struct rte_mempool *mbp;
	struct rte_ether_hdr eth_hdr;
	uint16_t nb_tx;
	uint16_t nb_pkt;
	uint16_t vlan_tci, vlan_tci_outer;
	uint64_t ol_flags = 0;
	struct rte_ether_addr addr;
	uint64_t start_tsc;
	uint64_t end_tsc;
	int rc;
#ifndef CONFIG_USE_DPDK_PMD
	struct rte_eth_dev *vrtl_eth_dev;
	struct uk_ethdev_private *dev_private;

	vrtl_eth_dev = &rte_eth_devices[port_id];
	UK_ASSERT(vrtl_eth_dev);
	dev_private = vrtl_eth_dev->data->dev_private;
	UK_ASSERT(dev_private);
#endif
	nb_pkt_per_burst = cnt;


	mbp = mp;
	rte_eth_macaddr_get(port_id, &addr);
	rte_ether_addr_copy(&arg_dst_mac, &eth_hdr.d_addr);
	rte_ether_addr_copy(&addr.addr_bytes[0], &eth_hdr.s_addr);
	eth_hdr.ether_type = rte_cpu_to_be_16(RTE_ETHER_TYPE_IPV4);

	start_tsc = rte_rdtsc();
	rc = rte_mempool_get_bulk(mbp, (void **)pkts_burst,
				  nb_pkt_per_burst);

#if 0
	printf("Mempool (%p) size: %d\n", mp,
	       rte_mempool_avail_count(mp));
#endif
	end_tsc = rte_rdtsc();

	pkt_stats.txpkt_buf_cycles[itr] += (end_tsc - start_tsc);
	if (rc == 0) {
		start_tsc = rte_rdtsc();
		for (nb_pkt = 0; nb_pkt < nb_pkt_per_burst; nb_pkt++) {
			/**
			 * TODO:
			 * Create UDP Packets.
			 */
#ifdef CONFIG_USE_DPDK_PMD
			if (unlikely(!pkt_burst_prepare_data(pkts_burst[nb_pkt], mbp,
							&eth_hdr, ol_flags,
							start_tsc,data, size))) {
				printf("%s: pkt burst prepare failed: %d\n",
				       __func__, nb_pkt);
				rte_mempool_put_bulk(mbp,
						(void **)&pkts_burst[nb_pkt],
						nb_pkt_per_burst - nb_pkt);
				break;
			}
#else
			if (unlikely(!pkt_burst_prepare_data(pkts_burst[nb_pkt], mbp,
							&eth_hdr, ol_flags,
							start_tsc,
							&uk_pkts_burst[nb_pkt], data, size))) {
				printf("%s: pkt burst prepare failed: %d\n",
				       __func__, nb_pkt);
				rte_mempool_put_bulk(mbp,
						(void **)&pkts_burst[nb_pkt],
						nb_pkt_per_burst - nb_pkt);
				break;
			}
#endif
		}
		end_tsc = rte_rdtsc();
		pkt_stats.total_txpkts_tries_pps[itr] += nb_pkt;
		pkt_stats.txpkt_gen_cycles[itr] += end_tsc - start_tsc;
	} else {
		start_tsc = rte_rdtsc();
		for (nb_pkt = 0; nb_pkt < nb_pkt_per_burst; nb_pkt++) {
			pkt = rte_mbuf_raw_alloc(mbp);
			if (pkt == NULL) {
				pkt_stats.total_txpkts_alloc_failed[itr] += nb_pkt_per_burst - nb_pkt;
				break;
			}
#ifdef CONFIG_USE_DPDK_PMD
			if (unlikely(!pkt_burst_prepare_data(pkt, mbp, &eth_hdr,
							ol_flags, start_tsc,
							data, size))) {
				rte_pktmbuf_free(pkt);
				break;
			}
#else
			if (unlikely(!pkt_burst_prepare_data(pkt, mbp, &eth_hdr,
							ol_flags, start_tsc,
							&uk_pkts_burst[nb_pkt],
							data, size))) {
				rte_pktmbuf_free(pkt);
				break;
			}
#endif
			pkts_burst[nb_pkt] = pkt;
		}
		end_tsc = rte_rdtsc();
		pkt_stats.txpkt_gen_cycles[itr] += end_tsc - start_tsc;
		pkt_stats.total_txpkts_tries_ra_pps[itr] += nb_pkt;
		start_tsc = rte_rdtsc();
	}

	start_tsc = rte_rdtsc();
#ifdef CONFIG_USE_DPDK_PMD
	nb_tx = rte_eth_tx_burst(port_id, 0, pkts_burst, nb_pkt);
#else
	nb_tx = nb_pkt;
	rc = uk_netdev_tx_burst(dev_private->netdev,
	                        0, &uk_pkts_burst[0], &nb_tx);
#endif /* CONFIG_USE_DPDK_PMD */
	if (nb_tx < nb_pkt) {
		pkt_stats.total_txpkts_tries_dropped[itr] += nb_pkt - nb_tx;
		rte_mempool_put_bulk(mbp,
				(void **)&pkts_burst[nb_tx],
				nb_pkt - nb_tx);
	}
	end_tsc = rte_rdtsc();
	pkt_stats.txpkt_xmit_cycles[itr] += (end_tsc - start_tsc);

	return nb_tx;
}


int pkt_burst_transmit(int port_id, struct rte_mempool *mp, int itr)
{
	struct rte_mbuf *pkts_burst[MAX_PKT_BURST];
	struct uk_netbuf *uk_pkts_burst[MAX_PKT_BURST];
	struct rte_port *txp;
	struct rte_mbuf *pkt;
	struct rte_mempool *mbp;
	struct rte_ether_hdr eth_hdr;
	uint16_t nb_tx;
	uint16_t nb_pkt;
	uint16_t vlan_tci, vlan_tci_outer;
	uint64_t ol_flags = 0;
	struct rte_ether_addr addr;
	uint64_t start_tsc;
	uint64_t end_tsc;
	int rc;
#ifndef CONFIG_USE_DPDK_PMD
	struct rte_eth_dev *vrtl_eth_dev;
	struct uk_ethdev_private *dev_private;

	vrtl_eth_dev = &rte_eth_devices[port_id];
	UK_ASSERT(vrtl_eth_dev);
	dev_private = vrtl_eth_dev->data->dev_private;
	UK_ASSERT(dev_private);
#endif


	mbp = mp;
	rte_eth_macaddr_get(port_id, &addr);
	rte_ether_addr_copy(&arg_dst_mac, &eth_hdr.d_addr);
	rte_ether_addr_copy(&addr.addr_bytes[0], &eth_hdr.s_addr);
	eth_hdr.ether_type = rte_cpu_to_be_16(RTE_ETHER_TYPE_IPV4);

	start_tsc = rte_rdtsc();
	rc = rte_mempool_get_bulk(mbp, (void **)pkts_burst,
				  nb_pkt_per_burst);

#if 0
	printf("Mempool (%p) size: %d\n", mp,
	       rte_mempool_avail_count(mp));
#endif
	end_tsc = rte_rdtsc();

	pkt_stats.txpkt_buf_cycles[itr] += (end_tsc - start_tsc);
	if (rc == 0) {
		start_tsc = rte_rdtsc();
		for (nb_pkt = 0; nb_pkt < nb_pkt_per_burst; nb_pkt++) {
			/**
			 * TODO:
			 * Create UDP Packets.
			 */
#ifdef CONFIG_USE_DPDK_PMD
			if (unlikely(!pkt_burst_prepare(pkts_burst[nb_pkt], mbp,
							&eth_hdr, ol_flags,
							start_tsc))) {
				printf("%s: pkt burst prepare failed: %d\n",
				       __func__, nb_pkt);
				rte_mempool_put_bulk(mbp,
						(void **)&pkts_burst[nb_pkt],
						nb_pkt_per_burst - nb_pkt);
				break;
			}
#else
			if (unlikely(!pkt_burst_prepare(pkts_burst[nb_pkt], mbp,
							&eth_hdr, ol_flags,
							start_tsc,
							&uk_pkts_burst[nb_pkt]))) {
				printf("%s: pkt burst prepare failed: %d\n",
				       __func__, nb_pkt);
				rte_mempool_put_bulk(mbp,
						(void **)&pkts_burst[nb_pkt],
						nb_pkt_per_burst - nb_pkt);
				break;
			}
#endif
		}
		end_tsc = rte_rdtsc();
		pkt_stats.total_txpkts_tries_pps[itr] += nb_pkt;
		pkt_stats.txpkt_gen_cycles[itr] += end_tsc - start_tsc;
	} else {
		start_tsc = rte_rdtsc();
		for (nb_pkt = 0; nb_pkt < nb_pkt_per_burst; nb_pkt++) {
			pkt = rte_mbuf_raw_alloc(mbp);
			if (pkt == NULL) {
				pkt_stats.total_txpkts_alloc_failed[itr] += nb_pkt_per_burst - nb_pkt;
				break;
			}
#ifdef CONFIG_USE_DPDK_PMD
			if (unlikely(!pkt_burst_prepare(pkt, mbp, &eth_hdr,
							ol_flags, start_tsc))) {
				rte_pktmbuf_free(pkt);
				break;
			}
#else
			if (unlikely(!pkt_burst_prepare(pkt, mbp, &eth_hdr,
							ol_flags, start_tsc,
							&uk_pkts_burst[nb_pkt]))) {
				rte_pktmbuf_free(pkt);
				break;
			}
#endif
			pkts_burst[nb_pkt] = pkt;
		}
		end_tsc = rte_rdtsc();
		pkt_stats.txpkt_gen_cycles[itr] += end_tsc - start_tsc;
		pkt_stats.total_txpkts_tries_ra_pps[itr] += nb_pkt;
		start_tsc = rte_rdtsc();
	}

	start_tsc = rte_rdtsc();
#ifdef CONFIG_USE_DPDK_PMD
	nb_tx = rte_eth_tx_burst(port_id, 0, pkts_burst, nb_pkt);
#else
	nb_tx = nb_pkt;
	rc = uk_netdev_tx_burst(dev_private->netdev,
	                        0, &uk_pkts_burst[0], &nb_tx);
#endif /* CONFIG_USE_DPDK_PMD */
	if (nb_tx < nb_pkt) {
		pkt_stats.total_txpkts_tries_dropped[itr] += nb_pkt - nb_tx;
		rte_mempool_put_bulk(mbp,
				(void **)&pkts_burst[nb_tx],
				nb_pkt - nb_tx);
	}
	end_tsc = rte_rdtsc();
	pkt_stats.txpkt_xmit_cycles[itr] += (end_tsc - start_tsc);

	return nb_tx;
}

#ifdef CONFIG_USE_DPDK_PMD
int pkt_burst_receive(int port_id, struct rte_mempool *mpool,
		      struct rte_mbuf **mbufs, int pkt_cnt, uint64_t *total_latency, int itr)
#else
int pkt_burst_receive(int port_id, struct rte_mempool *mpool,
		      struct uk_netbuf **mbufs, int pkt_cnt, uint64_t *total_latency, int itr)
#endif
{
	uint16_t nb_rx;
	__u16 cnt = 0;
	uint64_t pkt_start_tsc, start_tsc, end_tsc;
	uint64_t latency = 0;
	int i;
#ifndef CONFIG_USE_DPDK_PMD
	struct rte_eth_dev *vrtl_eth_dev;
	struct uk_ethdev_private *dev_private;
	nb_rx = pkt_cnt;
	int status;


	vrtl_eth_dev = &rte_eth_devices[port_id];
	UK_ASSERT(vrtl_eth_dev);
	dev_private = vrtl_eth_dev->data->dev_private;
	UK_ASSERT(dev_private);
#endif /* CONFIG_USE_DPDK_PMD */

	start_tsc  = rte_rdtsc();
	pkt_start_tsc = start_tsc;
#ifdef CONFIG_USE_DPDK_PMD
	nb_rx = rte_eth_rx_burst(port_id, 0, &mbufs[0],
				 pkt_cnt);
	end_tsc = rte_rdtsc();
	if (unlikely(nb_rx == 0)) {
		pkt_stats.rxpkt_zrecv_cycles[itr] += (end_tsc - start_tsc);
		return 0;
	}
	pkt_stats.rxpkt_recv_cycles[itr] += (end_tsc - start_tsc);
#else /* CONFIG_USE_DPDK_PMD */
	status = uk_netdev_rx_burst(dev_private->netdev, 0, &mbufs[0], &nb_rx);
	end_tsc = rte_rdtsc();
	if (nb_rx == 0) {
		pkt_stats.rxpkt_zrecv_cycles[itr] += (end_tsc - start_tsc);
		return 0;
	}
	pkt_stats.rxpkt_recv_cycles[itr] += (end_tsc - start_tsc);
#endif /* CONFIG_USE_DPDK_PMD */

	pkt_stats.rxpkt_process_cycles[itr] += (end_tsc - start_tsc);

	return nb_rx;
}

