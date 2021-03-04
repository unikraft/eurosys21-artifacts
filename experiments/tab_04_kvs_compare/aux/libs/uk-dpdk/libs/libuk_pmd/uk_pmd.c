/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2010-2014 Intel Corporation
 */

#include <rte_mbuf.h>
#include <rte_ethdev.h>
#include <rte_ethdev_driver.h>
#if CONFIG_PCI_BUS
#include <rte_pci.h>
#include <rte_bus_pci.h>
#endif /* CONFIG_PCI_BUS */
#include <rte_malloc.h>
#include <rte_memcpy.h>
#include <rte_memory.h>
#include <rte_ring.h>
#include <uk/netbuf.h>
#include <uk/netdev.h>
#include <uk/init.h>
#include <uk/print.h>
#include <uk/alloc.h>

#define MAX_PKT_BURST CONFIG_LIBUKNETDEV_MAX_PKT_BURST
#define DRIVER_NAME "uk_pmd"

static const char *uk_ethdev_driver_name = DRIVER_NAME;

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
	int queue_id;
	struct rte_eth_dev *dev;
};

static struct uk_ethdev_private *dev_list;
static struct rte_driver uk_netdev_driver = {.name = DRIVER_NAME};

extern int uk_netbuf_mbuf_dtor(struct uk_netbuf *nb);

static int uk_ethdev_start(struct rte_eth_dev *eth_dev __rte_unused)
{
	struct uk_ethdev_private *prv;
	int rc;

	UK_ASSERT(eth_dev);
	prv = eth_dev->data->dev_private;

	rc = uk_netdev_start(prv->netdev);
	if (rc < 0) {
		uk_pr_err("Failed to start the netdev: %p\n", prv->netdev);
		return rc;
	}

	eth_dev->data->dev_started = 1;
	eth_dev->data->dev_link.link_status = ETH_LINK_UP;

	return 0;
}

static void uk_ethdev_stop(struct rte_eth_dev *eth_dev __rte_unused)
{
	void *pkt = NULL;
	struct uk_ethdev_private *prv = eth_dev->data->dev_private;

	eth_dev->data->dev_link.link_status = ETH_LINK_DOWN;
	eth_dev->data->dev_started = 0;
	while (rte_ring_dequeue(prv->rx_queue, &pkt) != -ENOENT)
		rte_pktmbuf_free(pkt);

	while (rte_ring_dequeue(prv->tx_queue, &pkt) != -ENOENT)
		rte_pktmbuf_free(pkt);
}

static void uk_ethdev_close(struct rte_eth_dev *dev __rte_unused)
{}

static int uk_ethdev_configure(struct rte_eth_dev *dev)
{
	int rc = 0;
	struct uk_ethdev_private *priv;
	struct uk_netdev *ndev;
	struct uk_netdev_conf nconf = {0};
	struct uk_netdev_queue_info qi = {0};
	struct uk_hwaddr *hw_addr;
	uint16_t max_rx_desc = 0, max_tx_desc = 0;

	UK_ASSERT(dev);
	priv = dev->data->dev_private;
	ndev =  priv->netdev;

	printf("%s: Configuring ethdev %p\n", __func__, dev);

	nconf.nb_rx_queues = dev->data->nb_rx_queues;
	nconf.nb_tx_queues = dev->data->nb_tx_queues;

	rc = uk_netdev_configure(ndev, &nconf);

	uk_netdev_rxq_info_get(ndev, 0, &qi);
	priv->max_rx_desc = qi.nb_max;
	uk_netdev_txq_info_get(ndev, 0, &qi);
	priv->max_tx_desc = qi.nb_max;

	rc = rte_eth_dev_adjust_nb_rx_tx_desc(dev->data->port_id, &max_rx_desc,
					      &max_tx_desc);
	hw_addr = uk_netdev_hwaddr_get(ndev);
	memcpy(dev->data->mac_addrs, hw_addr,
			sizeof(*dev->data->mac_addrs));

	return rc;
}

static void uk_ethdev_info_get(struct rte_eth_dev *dev,
			       struct rte_eth_dev_info *dev_info)
{
	struct uk_ethdev_private *priv_dev;
	struct uk_netdev *nd;
	struct uk_netdev_info nd_info = {0};

	UK_ASSERT(dev);

	priv_dev =  dev->data->dev_private;
	nd = priv_dev->netdev;

	uk_netdev_info_get(nd, &nd_info);

	uk_pr_info("rx_queue: %d, txqueue: %d\n", nd_info.max_rx_queues,
			nd_info.max_tx_queues);
	dev_info->driver_name = uk_ethdev_driver_name;
	dev_info->max_mtu = nd_info.max_mtu;
	dev->data->nb_rx_queues = nd_info.max_rx_queues;
	dev->data->nb_tx_queues = nd_info.max_tx_queues; 
	dev_info->max_rx_queues = nd_info.max_rx_queues;
	dev_info->max_tx_queues = nd_info.max_tx_queues;
	dev_info->max_rx_pktlen = (uint32_t)2048;
	dev_info->rx_desc_lim.nb_max = priv_dev->max_rx_desc;
	dev_info->tx_desc_lim.nb_max = priv_dev->max_tx_desc;

	/**
	 * Need this information for netbuf
	 */
	priv_dev->nb_encap_rx = nd_info.nb_encap_rx;
	priv_dev->nb_encap_tx = nd_info.nb_encap_tx;
}

static uint16_t uk_pmd_alloc_rxpkts(void *argp, struct uk_netbuf *pkts[], uint16_t count)
{
	int rc;
	struct rte_mempool *mp;
	int pkt_cnt, i;
	struct rte_mbuf *mpkts[CONFIG_LIBUKNETDEV_MAX_PKT_BURST];

	UK_ASSERT(argp && pkts);

	mp = (struct rte_mempool *) argp;
	pkt_cnt = count;
	
	do {
		rc = rte_mempool_get_bulk(mp, mpkts, pkt_cnt);
		if (rc == 0)
			goto exit;

#if 0
		printf("pool: %p mempool_size: %d\n", mp,
			rte_mempool_avail_count(mp));
#endif
		pkt_cnt = pkt_cnt >> 1;
	} while (pkt_cnt > 0);
	return 0;
exit:
	//printf("preparing packet: %d\n", pkt_cnt);
	for (i = 0; i < pkt_cnt; i++) {
		struct rte_mbuf *mbuf = mpkts[i];
		rte_pktmbuf_reset_headroom(mbuf);
		pkts[i] = (struct uk_netbuf *)mbuf->userdata;
		UK_ASSERT(mbuf->userdata);
		uk_netbuf_prepare_buf(pkts[i], sizeof(struct uk_netbuf) +
				      mbuf->buf_len + sizeof(*mbuf) +
				      mbuf->priv_size,
				      mbuf->data_off,
				sizeof(*mbuf) + mbuf->priv_size,
				uk_netbuf_mbuf_dtor);
		pkts[i]->len = pkts[i]->buflen - mbuf->data_off;
	}
	return pkt_cnt;
}

static int uk_ethdev_rx_queue_setup(struct rte_eth_dev *dev,
				    uint16_t rx_queue_id,
				    uint16_t nb_rx_desc __rte_unused,
				    unsigned int socket_id,
				    const struct rte_eth_rxconf *rx_conf __rte_unused,
				    struct rte_mempool *mb_pool __rte_unused)
{
	struct uk_ethdev_private *priv_dev;
	struct uk_netdev *nd;
	struct uk_netdev_rxqueue_conf nconf = {0};
	struct uk_ethdev_queue *queue;
	int rc;

	UK_ASSERT(dev);

	priv_dev = dev->data->dev_private;
	nd = priv_dev->netdev;
	nconf.a = uk_alloc_get_default();
	nconf.callback = priv_dev;
	nconf.alloc_rxpkts = uk_pmd_alloc_rxpkts;
	nconf.alloc_rxpkts_argp = mb_pool;

	queue = uk_zalloc(nconf.a, sizeof(*queue));
	if (!queue) {
		uk_pr_err("Failed to allocate rx queue\n");
		return -ENOMEM;
	}

	uk_pr_info("%s: Configure the device queue id: %d\n", __func__,
		   rx_queue_id);

	rc = uk_netdev_rxq_configure(nd, rx_queue_id, 0, &nconf);
	if (rc < 0) {
		uk_pr_err("Failed to configure the rx queue: %d\n",
			  rx_queue_id);
		return rc;
	}
	queue->port_id = dev->data->port_id;
	queue->queue_id = rx_queue_id;
	queue->dev = dev;
	dev->data->rx_queues[rx_queue_id] = queue;

	return 0;
}

static int uk_ethdev_tx_queue_setup(struct rte_eth_dev *dev,
				 uint16_t tx_queue_id,
				 uint16_t nb_tx_desc __rte_unused,
				 unsigned int socket_id,
				 const struct rte_eth_txconf *tx_conf __rte_unused)
{
	struct uk_ethdev_private *priv_dev;
	struct uk_netdev *nd;
	struct uk_ethdev_queue *queue;
	struct uk_netdev_txqueue_conf nconf = {0};
	int rc;

	UK_ASSERT(dev);

	priv_dev = dev->data->dev_private;
	nd = priv_dev->netdev;

	nconf.a = uk_alloc_get_default();

	queue = uk_zalloc(nconf.a, sizeof(*queue));
	if (!queue) {
		uk_pr_err("Failed to allocate tx queue\n");
		return -ENOMEM;
	}

	rc = uk_netdev_txq_configure(nd, tx_queue_id, 0, &nconf);
	if (rc < 0) {
		uk_pr_err("Failed to configure the tx queue: %d\n",
			  tx_queue_id);
		goto free_mem;
	}

	uk_pr_info("%s: configure txq id:%d, with ref: %p\n", __func__,
		   tx_queue_id, queue);
	queue->port_id = dev->data->port_id;
	queue->queue_id = tx_queue_id;
	queue->dev = dev;
	dev->data->tx_queues[tx_queue_id] = queue;

	return 0;

free_mem:
	uk_free(nconf.a, queue);
	return rc;
}

static void uk_ethdev_rx_queue_release(void *q __rte_unused)
{
}

static void uk_ethdev_tx_queue_release(void *q __rte_unused)
{
}

static int uk_ethdev_link_update(struct rte_eth_dev *bonded_eth_dev,
					 int wait_to_complete __rte_unused)
{
	if (!bonded_eth_dev->data->dev_started)
		bonded_eth_dev->data->dev_link.link_status = ETH_LINK_DOWN;

	return 0;
}

static int uk_ethdev_stats_get(struct rte_eth_dev *dev,
			       struct rte_eth_stats *stats)
{
	struct uk_ethdev_private *dev_private = dev->data->dev_private;

	if (stats)
		rte_memcpy(stats, &dev_private->eth_stats, sizeof(*stats));

	return 0;
}

static void uk_ethdev_stats_reset(struct rte_eth_dev *dev)
{
	struct uk_ethdev_private *dev_private = dev->data->dev_private;
	void *pkt = NULL;

	while (rte_ring_dequeue(dev_private->tx_queue, &pkt) == -ENOBUFS)
			rte_pktmbuf_free(pkt);

	/* Reset internal statistics */
	memset(&dev_private->eth_stats, 0, sizeof(dev_private->eth_stats));
}

static void uk_ethdev_promiscuous_mode_enable(struct rte_eth_dev *dev __rte_unused)
{}

static void uk_ethdev_promiscuous_mode_disable(struct rte_eth_dev *dev __rte_unused)
{}

static int uk_ethdev_mac_address_set(__rte_unused struct rte_eth_dev *dev,
				     __rte_unused struct rte_ether_addr *addr)
{
	return 0;
}

static const struct eth_dev_ops uk_ethdev_default_dev_ops = {
	.dev_configure = uk_ethdev_configure,
	.dev_start = uk_ethdev_start,
	.dev_stop = uk_ethdev_stop,
	.dev_close = uk_ethdev_close,
	.dev_infos_get = uk_ethdev_info_get,
	.rx_queue_setup = uk_ethdev_rx_queue_setup,
	.tx_queue_setup = uk_ethdev_tx_queue_setup,
	.rx_queue_release = uk_ethdev_rx_queue_release,
	.tx_queue_release = uk_ethdev_tx_queue_release,
	.link_update = uk_ethdev_link_update,
	.mac_addr_set = uk_ethdev_mac_address_set,
	.stats_get = uk_ethdev_stats_get,
	.stats_reset = uk_ethdev_stats_reset,
	.promiscuous_enable = uk_ethdev_promiscuous_mode_enable,
	.promiscuous_disable = uk_ethdev_promiscuous_mode_disable
};

static uint16_t uk_ethdev_rx_burst(void *queue,
				   struct rte_mbuf **bufs,
				   uint16_t nb_pkts)
{
	struct rte_eth_dev *vrtl_eth_dev;
	struct uk_ethdev_private *dev_private;
	struct uk_ethdev_queue *rxq;
	struct uk_netbuf *nb[MAX_PKT_BURST];
	int rx_burst_size = (nb_pkts > MAX_PKT_BURST)? MAX_PKT_BURST:nb_pkts;
	int rx_count = 0, i, rc, idx;
	struct rte_mbuf *mbuf;

	UK_ASSERT(queue && bufs);

	rxq = (struct uk_ethdev_private *) queue;
	vrtl_eth_dev = &rte_eth_devices[rxq->port_id];
	dev_private = vrtl_eth_dev->data->dev_private;
	UK_ASSERT(dev_private);
	nb_pkts = (nb_pkts > MAX_PKT_BURST)? MAX_PKT_BURST:nb_pkts;

	if (unlikely(!vrtl_eth_dev->data->dev_link.link_status)) {
		return 0;
	} else {
		while (rx_count < nb_pkts) {
			rx_burst_size = nb_pkts - rx_count;
			rc = uk_netdev_rx_burst(dev_private->netdev,
					rxq->queue_id, &nb[rx_count],
					&rx_burst_size);
			idx = rx_count;
			for (i = idx; i < idx + rx_burst_size; i++) {
				rx_count++;
				UK_ASSERT(nb[i]);
				mbuf = nb[i]->priv;
				//printf("%s: mbuf %p\n", __func__, mbuf);
				/**
				 * TODO: Fill in the mbuf for packet processing
				 */
				mbuf->port = rxq->port_id;
				//mbuf->data_off = nb[i]->data - nb[i]->buf;
				mbuf->ol_flags = 0;
				mbuf->vlan_tci = 0;

				mbuf->pkt_len = nb[i]->len;
				mbuf->data_len = nb[i]->len;
				bufs[i] = mbuf;

			}

			if (!uk_netdev_status_more(rc))
				break;
		}
	}
	/* increments ipackets count */
	dev_private->eth_stats.ipackets += rx_count;

	/* increments ibytes count */
	for (i = 0; i < rx_count; i++)
		dev_private->eth_stats.ibytes += rte_pktmbuf_pkt_len(bufs[i]);

	return rx_count;
}

static uint16_t uk_ethdev_tx_burst(void *queue, struct rte_mbuf **bufs,
				   uint16_t nb_pkts)
{
	struct uk_ethdev_queue *txq = queue;

	struct rte_eth_dev *vrtl_eth_dev;
	struct uk_ethdev_private *dev_private;
	struct uk_netbuf *nb[CONFIG_LIBUKNETDEV_MAX_PKT_BURST];
	struct rte_mempool_objhdr *mp_obj_hdr;
	int i, rc;
	int count = nb_pkts;

	UK_ASSERT(queue);

	vrtl_eth_dev = &rte_eth_devices[txq->port_id];
	UK_ASSERT(vrtl_eth_dev);
	dev_private = vrtl_eth_dev->data->dev_private;
	UK_ASSERT(dev_private);

	if (!vrtl_eth_dev->data->dev_link.link_status) {
		nb_pkts = 0;
	} else {
		uk_pr_debug("enqueue %d bufs on port: %d\n", nb_pkts,
			   txq->port_id);

		for (i = 0; i < nb_pkts; i++) {
			nb[i] = (struct uk_netbuf *) (((uintptr_t) bufs[i]) -
					sizeof(struct uk_netbuf));

			nb[i]->len = bufs[i]->pkt_len;
			uk_pr_debug("Sending packet: %d netbuf: %p on queue: %d len: %d mbuf_len: %d\n", i, nb[i], txq->queue_id, nb[i]->len, bufs[i]->pkt_len);
		}

		rc = uk_netdev_tx_burst(dev_private->netdev,
				      txq->queue_id, &nb[0], &count);
		UK_ASSERT(rc >= 0);
		uk_pr_debug("bufs with dev_priv:%p txq: %p\n",
				dev_private, dev_private->tx_queue);
	}

	/* increment opacket count */
	dev_private->eth_stats.opackets += count;

	/* increment obytes count */
	for (i = 0; i < count; i++)
		dev_private->eth_stats.obytes += rte_pktmbuf_pkt_len(bufs[i]);

	return count;
}

void uk_ethdev_set_link_status(uint16_t port_id, uint8_t link_status)
{
	struct rte_eth_dev *vrtl_eth_dev = &rte_eth_devices[port_id];

	vrtl_eth_dev->data->dev_link.link_status = link_status;
}

void uk_ethdev_simulate_link_status_interrupt(uint16_t port_id,
					      uint8_t link_status)
{
	struct rte_eth_dev *vrtl_eth_dev = &rte_eth_devices[port_id];

	vrtl_eth_dev->data->dev_link.link_status = link_status;

	_rte_eth_dev_callback_process(vrtl_eth_dev, RTE_ETH_EVENT_INTR_LSC,
				      NULL);
}

int uk_ethdev_add_mbufs_to_rx_queue(uint16_t port_id,
				struct rte_mbuf **pkt_burst, int burst_length)
{
	struct rte_eth_dev *vrtl_eth_dev = &rte_eth_devices[port_id];
	struct uk_ethdev_private *dev_private =
			vrtl_eth_dev->data->dev_private;

	return rte_ring_enqueue_burst(dev_private->rx_queue, (void **)pkt_burst,
			burst_length, NULL);
}

int uk_ethdev_get_mbufs_from_tx_queue(uint16_t port_id,
				      struct rte_mbuf **pkt_burst,
				      int burst_length)
{
	struct uk_ethdev_private *dev_private;
	struct rte_eth_dev *vrtl_eth_dev = &rte_eth_devices[port_id];

	dev_private = vrtl_eth_dev->data->dev_private;
	return rte_ring_dequeue_burst(dev_private->tx_queue, (void **)pkt_burst,
		burst_length, NULL);
}

/**
 * Register a ethdev with DPDK
 */
static int uk_ethdev_create(struct uk_netdev *dev, const char *name,
			    uint8_t socket_id,
			    uint8_t isr_support)
{
#if CONFIG_PCI_BUS
	struct rte_pci_device *pci_dev = NULL;
	struct rte_pci_driver *pci_drv = NULL;
	struct rte_pci_id *id_table = NULL;
#endif
	struct rte_eth_dev *eth_dev = NULL;
	struct uk_ethdev_private *dev_private = NULL;
	char name_buf[RTE_RING_NAMESIZE];

#ifdef CONFIG_PCI_BUS
	/* now do all data allocation - for eth_dev structure, dummy pci driver
	 * and internal (dev_private) data
	 */
	pci_dev = rte_zmalloc_socket(name, sizeof(*pci_dev), 0, socket_id);
	if (pci_dev == NULL)
		goto err;

	pci_drv = rte_zmalloc_socket(name, sizeof(*pci_drv), 0, socket_id);
	if (pci_drv == NULL)
		goto err;

	id_table = rte_zmalloc_socket(name, sizeof(*id_table), 0, socket_id);
	if (id_table == NULL)
		goto err;
	id_table->device_id = 0xBEEF;
#endif /* CONFIG_PCI_BUS */

	dev_private = rte_zmalloc_socket(name, sizeof(*dev_private), 0, socket_id);
	if (dev_private == NULL)
		goto err;

	dev_private->netdev = dev; 

#ifdef CONFIG_UK_NETDEV_RING
	snprintf(name_buf, sizeof(name_buf), "%s_rxQ", name);
	dev_private->rx_queue = rte_ring_create(name_buf, MAX_PKT_BURST, socket_id,
			0);
	if (dev_private->rx_queue == NULL)
		goto err;

	snprintf(name_buf, sizeof(name_buf), "%s_txQ", name);
	dev_private->tx_queue = rte_ring_create(name_buf, MAX_PKT_BURST, socket_id,
			0);
	if (dev_private->tx_queue == NULL)
		goto err;
#endif /* CONFIG_UK_NETDEV_RING */

	/* reserve an ethdev entry */
	eth_dev = rte_eth_dev_allocate(name);
	if (eth_dev == NULL)
		goto err;

#ifdef CONFIG_PCI_BUS
	pci_dev->device.numa_node = socket_id;
	pci_dev->device.name = eth_dev->data->name;
	pci_drv->driver.name = uk_ethdev_driver_name;
	pci_drv->id_table = id_table;

	if (isr_support)
		pci_drv->drv_flags |= RTE_PCI_DRV_INTR_LSC;
	else
		pci_drv->drv_flags &= ~RTE_PCI_DRV_INTR_LSC;


#endif /* CONFIG_PCI_BUS */
	eth_dev->device = &dev_private->dev;
	eth_dev->device->driver = &uk_netdev_driver;

	eth_dev->data->nb_rx_queues = (uint16_t)1;
	eth_dev->data->nb_tx_queues = (uint16_t)1;

	eth_dev->data->dev_link.link_status = ETH_LINK_DOWN;
	eth_dev->data->dev_link.link_speed = ETH_SPEED_NUM_10G;
	eth_dev->data->dev_link.link_duplex = ETH_LINK_FULL_DUPLEX;

	eth_dev->data->mac_addrs = rte_zmalloc(name, RTE_ETHER_ADDR_LEN, 0);
	if (eth_dev->data->mac_addrs == NULL)
		goto err;

#ifdef CONFIG_DEFAULT_HWADDR
	memcpy(eth_dev->data->mac_addrs, mac_addr,
			sizeof(*eth_dev->data->mac_addrs));
#endif /* CONFIG_DEFAULT_HWADDR */

	eth_dev->data->dev_started = 0;
	eth_dev->data->promiscuous = 0;
	eth_dev->data->scattered_rx = 0;
	eth_dev->data->all_multicast = 0;

	eth_dev->data->dev_private = dev_private;

	/* Copy default device operation functions */
	dev_private->dev_ops = uk_ethdev_default_dev_ops;
	eth_dev->dev_ops = &dev_private->dev_ops;

#ifdef CONFIG_PCI_BUS
	pci_dev->device.driver = &pci_drv->driver;
	eth_dev->device = &pci_dev->device;
#endif /* CONFIG_PCI_BUS */

	eth_dev->rx_pkt_burst = uk_ethdev_rx_burst;
	eth_dev->tx_pkt_burst = uk_ethdev_tx_burst;

	rte_eth_dev_probing_finish(eth_dev);

	return eth_dev->data->port_id;

err:
#ifdef CONFIG_PCI_BUS */
	rte_free(pci_dev);
	rte_free(pci_drv);
	rte_free(id_table);
#endif /* CONFIG_PCI_BUS */
	rte_free(dev_private);

	return -1;
}

/**
 * Add the uk_netdev happens as a part of the inittab
 */
int eal_uknetdev_init(void)
{
	int cnt = 0, i, rc;
	struct uk_netdev *dev;


#define RTE_ETHDEV_NAMESIZE  5
	char name[RTE_ETHDEV_NAMESIZE];

	cnt = uk_netdev_count();

	for (i = 0; i < cnt; i++) {
		dev = uk_netdev_get(i);
		if (!dev)
			continue;
		uk_pr_info(DRIVER_NAME": Registered netdev id %d @ %p\n", i, dev);
		snprintf(name, sizeof(name), "uk%02d", i);
		rc = uk_ethdev_create(dev, name, eal_cpu_socket_id(), 0);
		if (rc < 0) {
			uk_pr_err("Failed to create the ethdev\n");
			goto err_exit;
		}
	}
	return 0;

err_exit:
	return -1;
}
