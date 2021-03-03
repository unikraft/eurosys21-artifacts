#include <testsuite.h>
#include <test_assert.h>
#include <stdio.h>
#include <stdint.h>
#include <uk/netdev.h>
#include <uk/netdev_core.h>
#include <uk/alloc.h>
#include <uk/semaphore.h>
#include <uk/arch/types.h>

#define NET_DEVICE_COUNT   2
#define EXPECTED_PKT       (0x100)
#define NET_DATA_SIZE      (1530)
#define DESC_COUNT         (0x1000)

static struct uk_alloc *a = NULL;
static struct uk_sched *s = NULL;
static struct uk_netdev *netdevice[NET_DEVICE_COUNT] = {0};
static struct uk_hwaddr  hw_addrs[NET_DEVICE_COUNT];
static __u16  desc_cnt[NET_DEVICE_COUNT] = {0};
static struct uk_netbuf *spare_buf[NET_DEVICE_COUNT];
static __atomic pkt_cnt[NET_DEVICE_COUNT];
static  struct uk_semaphore sem_flag;
static struct uk_netdev_info dev_info[NET_DEVICE_COUNT] = {0};

static void netdev_test_callback(struct uk_netdev *dev,
					uint16_t queue_id, void *cookie);

#if 0
static void netdev_arp_response_frame(struct uk_netbuf *buf)
{
	uint16_t swap;
	int i = 0;
	/* swap source and destination MAC */
	for (i = 0; i < 3; i++) {
		swap = *(((uint16_t *)buf->data) + i);
		uk_pr_info("%04x, %04x\n", swap,
			*(((uint16_t *)buf->data) + i + 3));
	}
}

static void netdev_form_reply(struct uk_netbuf *buf)
{
	__u16 ethertype = *(__u16 *)(buf->data + 12);
	uk_pr_info("Ethertype %04x\n", ethertype);
	netdev_arp_response_frame(buf);
}
#endif /* ZERO */

static void netdev_test_data_tx(uint16_t queue_id,
				struct uk_netbuf *buf, int instance)
{
	struct uk_netbuf *sendbuf = NULL;
	int rc = 0;
	uk_pr_info("Allocating send buffer\n");
	sendbuf = uk_netbuf_alloc_buf(a, NET_DATA_SIZE, 0, 0, NULL);
	TEST_NOT_NULL(sendbuf);
	uk_pr_info("Copying data of length %d\n", buf->len);
	memcpy(sendbuf->data, buf->data, buf->len);
	sendbuf->len = buf->len;
	UK_ASSERT(!sendbuf->prev);
	uk_pr_info("Sending(%d) data of length %d\n",
		   instance, sendbuf->len);
	rc = uk_netdev_tx_one(netdevice[instance], queue_id, sendbuf);
	TEST_EXPR(rc == 2);
}

static uint16_t netdev_test_buf_alloc(void *argp, struct uk_netbuf *pkts[],
				      uint16_t count)
{
	int i;

	for (i = 0; i < count; ++i) {
		pkts[i] = uk_netbuf_alloc_buf(a, NET_DATA_SIZE, 0, 0, NULL);
		if (!pkts[i]) {
			/* we run ut of memory */
			return i;
		}
		pkts[i]->len = pkts[i]->buflen;
	}
	return count;

}

static void netdev_test_callback(struct uk_netdev *dev, uint16_t queue_id,
				 void *cookie)
{
	struct uk_netbuf *buf = NULL;
	struct uk_netbuf *sendbuf = NULL;
	__u16 count = 0;
	int rc = 0;
	static int rcv_stat = 0;
	int instance = (int) cookie;
	uk_pr_info("Recv Identifier %d %p\n", instance, buf);

	do {
		rc = uk_netdev_rx_one(dev, queue_id, &buf);
		if (rc < 0) {
			uk_pr_err("Error receiving packet\n");
			TEST_EXPR(rc >= 0);
		}

		uk_pr_info("Receive return code %d\n", rc);
		if (rc > 0) {
			if (uk_netdev_status_successful(rc)) {
				TEST_NOT_NULL(buf);
			}
		} else if (rc == 0) {
			sleep(1);
			continue;
		}

		uk_pr_info("instance %d\n", instance);
		netdev_test_data_tx(queue_id, buf,  1 - instance);
		uk_pr_info("packet processed %d\n", ++rcv_stat);
	} while (rcv_stat < EXPECTED_PKT);

	uk_pr_info("out of loop %d\n", rcv_stat);
	ukarch_inc(&pkt_cnt[instance].counter);
	if (pkt_cnt[instance].counter == EXPECTED_PKT) {
		uk_pr_info("uping the sem\n");
		uk_semaphore_up(&sem_flag);
	}
}

void netdev_init(uint32_t *count)
{
	a =  uk_alloc_get_default();
	*count  = uk_netdev_count();
}

void netdev_test_init()
{
	int count = 0;
	netdev_init(&count);
	TEST_NOT_ZERO_CHK(count);
}

#ifdef CONFIG_UKNETDEVTEST_FETCH
void netdev_test_fetch(int count)
{
	int i = 0;
	struct uk_netdev *dev;
	for (i = 0; i < count; i++) {
		uk_pr_info("Fetching index %d\n", i);
		dev = uk_netdev_get(i);
		TEST_NOT_NULL(dev);
		netdevice[i] = dev;
	}
}
#endif /* CONFIG_UKNETDEVTEST_FETCH */

#ifdef CONFIG_UKNETDEVTEST_CONFIGURE
void netdev_test_invalid_configure(int instance)
{
	struct uk_netdev_conf conf = {0};
	int rc = 0;
	conf.nb_rx_queues = 5;
	conf.nb_tx_queues = 5;
	rc = uk_netdev_configure(netdevice[instance], &conf);
	TEST_EXPR(rc < 0);
}
#endif /* CONFIG_UKNETDEVTEST_CONFIGURE */


#ifdef CONFIG_UKNETDEVTEST_CONFIGURE
void netdev_test_configure(int instance)
{
	struct uk_netdev_info *info = &dev_info[instance];
	int rc = 0;
	struct uk_netdev_conf conf;

	uk_netdev_info_get(netdevice[instance], info);
	TEST_EXPR(info->nb_encap_rx >= 0);
	TEST_EXPR(info->nb_encap_tx >= 0);
	uk_pr_info("RX Encap %d: TX Encap %d\n", info->nb_encap_rx, info->nb_encap_tx);

	conf.nb_rx_queues = info->max_rx_queues;
	conf.nb_tx_queues = info->max_tx_queues;
	uk_pr_info("Configuring the device %d\n", instance);
	
	rc = uk_netdev_configure(netdevice[instance], &conf);
	TEST_ZERO_CHK(rc);
}
#endif /* CONFIG_UKNETDEVTEST_CONFIGURE */

#ifdef CONFIG_UKNETDEVTEST_CONFIGURE_RX
void netdev_test_rx_queue_configure(int instance)
{
	struct uk_netdev_rxqueue_conf conf = {0};
	int rc = 0;
	conf.s = uk_sched_get_default();
	conf.a = uk_alloc_get_default();
	conf.callback = netdev_test_callback;
	conf.callback_cookie = (void *) instance;
	conf.alloc_rxpkts = netdev_test_buf_alloc;
	conf.alloc_rxpkts_argp = (void *)instance;
	rc = uk_netdev_rxq_configure(netdevice[instance], 0, DESC_COUNT,
			&conf);
	TEST_ZERO_CHK(rc);

	rc = uk_netdev_rxq_configure(netdevice[instance], 0, 1000,
			&conf);
}
#endif /* CONFIG_UKNETDEVTEST_CONFIGURE_RX */

#ifdef CONFIG_UKNETDEVTEST_CONFIGURE_TX
void netdev_test_tx_queue_configure(int instance)
{
	struct uk_netdev_txqueue_conf conf = {0};
	int rc = 0;
	conf.a = uk_alloc_get_default();
	rc = uk_netdev_txq_configure(netdevice[instance], 0, DESC_COUNT,
			&conf);
	TEST_ZERO_CHK(rc);
}
#endif /* CONFIG_UKNETDEVTEST_CONFIGURE_TX */

#ifdef CONFIG_UKNETDEVTEST_START
void netdev_test_start(int instance)
{
	int rc = 0;
	uk_pr_info("Starting the netdevice\n");
	rc = uk_netdev_start(netdevice[instance]);
	TEST_ZERO_CHK(rc);
}
#endif /* CONFIG_UKNETDEVTEST_START */

#ifdef CONFIG_UKNETDEVTEST_RX_INTR
void netdev_test_rxq_intr_enable(int instance)
{
	struct uk_netdev_rxqueue_conf conf = {0};
	int rc = 0;
	conf.s = uk_sched_get_default();
	conf.a = uk_alloc_get_default();
	rc = uk_netdev_rxq_intr_enable(netdevice[instance], 0);
	TEST_ZERO_CHK(rc);
}
#endif /* CONFIG_UKNETDEVTEST_RX_INTR */

#ifdef CONFIG_UKNETDEVTEST_MTU
static void netdev_test_mtu(int instance)
{
	int rc;
	int mtu = (1 << 8);
	int dft;

	rc = uk_netdev_mtu_get(netdevice[instance]);
	TEST_EXPR(rc > 0);
	TEST_SNUM_EQUALS(rc, 1500);

	dft = rc;

	rc = uk_netdev_mtu_set(netdevice[instance], mtu);
	TEST_SNUM_EQUALS(rc, 0);
	
	rc = uk_netdev_mtu_get(netdevice[instance]);
	TEST_EXPR(rc > 0);
	TEST_SNUM_EQUALS(rc, mtu);

	rc = uk_netdev_mtu_set(netdevice[instance], dft);
	TEST_SNUM_EQUALS(rc, 0);
}
#endif /* CONFIG_UKNETDEVTEST_MTU */

#ifdef CONFIG_UKNETDEVTEST_HWADDR
static void netdev_test_hwaddr_config(int instance)
{
	int rc;
	char hw_addr_fmt[6] = {0x2, 0, 0, 0, 0};
	struct uk_hwaddr *addr = NULL;
	int i = instance;


	memcpy(hw_addrs[i].addr_bytes, hw_addr_fmt, 5);
	hw_addrs[i].addr_bytes[5] = 1 + i;

	addr = uk_netdev_hwaddr_get(netdevice[instance]);
	TEST_EXPR(addr);
	TEST_BYTES_EQUALS(addr->addr_bytes, hw_addrs[i].addr_bytes, 6);

	memcpy(hw_addrs[i].addr_bytes, hw_addr_fmt, 5);
	hw_addrs[i].addr_bytes[5] = 100 + i;

	rc = uk_netdev_hwaddr_set(netdevice[instance], &hw_addrs[i]);
	TEST_SNUM_EQUALS(rc, 0);

	addr = uk_netdev_hwaddr_get(netdevice[instance]);
	TEST_EXPR(addr);
	TEST_BYTES_EQUALS(addr->addr_bytes, hw_addrs[i].addr_bytes, 6);
}
#endif /* CONFIG_UKNETDEVTEST_HWADDR */

#ifdef CONFIG_UKNETDEVTEST_RX_DESC
void netdev_test_add_recv_desc_append(int instance)
{
	__u16 count = 1;
	int rc = 0, i = 0;
	struct uk_netbuf *buf = NULL;
	struct uk_netdev_info *conf = &dev_info[instance];
	uk_pr_info("RX queue hdr %d\n", conf->nb_encap_rx);

	/**
	 * Test a single buffer allocation.
	 */
	for (i = 0; i < DESC_COUNT; i++) {
	}
}
#endif /* CONFIG_UKNETDEVTEST_RX_DESC */

static void netdev_receive_prepare(int instance __maybe_unused)
{
#ifndef CONFIG_UKNETDEVTEST_POLL
	uk_pr_info("Sleeping %ld\n", sem_flag.count);
	uk_semaphore_down(&sem_flag);
	uk_pr_info("Waking up %ld\n", sem_flag.count);
#else
	netdev_test_callback(netdevice[instance], 0, instance);
#endif /* CONFIG_UKNETDEVTEST_POLL */
}

static int test_netdev(void)
{
	uint32_t count = 0;
	int i;

	/* Hold the main thread */
	uk_semaphore_init(&sem_flag, 0);
	uk_pr_info("Semaphore %ld\n", sem_flag.count);
	pkt_cnt[0].counter = 0;
	pkt_cnt[1].counter = 0;
#ifdef CONFIG_UKNETDEVTEST_INIT
	netdev_test_init();
	uk_pr_info("Semaphore after init %ld\n", sem_flag.count);
#endif /* CONFIG_UKNETDEVTEST_INIT */

#ifdef CONFIG_UKNETDEVTEST_FETCH
	netdev_init(&count);
	uk_pr_info("Device Count %d\n", count);
	netdev_test_fetch(count);
#endif /* CONFIG_UKNETDEVTEST_FETCH */

#ifdef CONFIG_UKNETDEVTEST_CONFIGURE
	for (i = 0; i < count; i++) {
		netdev_test_configure(i);
	}
	uk_pr_info("Semaphore after configure %ld\n", sem_flag.count);
	netdev_test_invalid_configure(0);
#endif /* CONFIG_UKNETDEVTEST_CONFIGURE */

#ifdef CONFIG_UKNETDEVTEST_HWADDR
	for (i = 0; i < count; i++) {
		netdev_test_hwaddr_config(i);
	}
#endif /* CONFIG_UKNETDEV_HWADDR */

#ifdef CONFIG_UKNETDEVTEST_MTU
	for (i = 0; i < count; i++) {
		netdev_test_mtu(i);
	}
#endif /* CONFIG_UKNETDEVTEST_MTU */
#ifdef CONFIG_UKNETDEVTEST_CONFIGURE_RX
	for (i = 0; i < count; i++) {
		netdev_test_rx_queue_configure(i);
	}
	uk_pr_info("Semaphore after rx configure %ld\n", sem_flag.count);
#endif /* CONFIG_UKNETDEVTEST_CONFIGURE_RX */
#ifdef CONFIG_UKNETDEVTEST_CONFIGURE_TX
	for (i = 0; i < count; i++) {
		netdev_test_tx_queue_configure(i);
	}
	uk_pr_info("Semaphore after tx configure %ld\n", sem_flag.count);
#endif /* CONFIG_UKNETDEVTEST_CONFIGURE_TX */

#ifdef CONFIG_UKNETDEVTEST_START
	for (i = 0; i < count; i++) {
		netdev_test_start(i);
	}
#endif /* CONFIG_UKNETDEVTEST_START */

#ifdef CONFIG_UKNETDEVTEST_RX_INTR
	netdev_test_rxq_intr_enable(0);
	uk_pr_info("Semaphore after intr enable %ld\n", sem_flag.count);
	netdev_test_rxq_intr_enable(1);
	uk_pr_info("Semaphore after intr enable %ld\n", sem_flag.count);
	uk_pr_info("Enabling interrupt\n");
#endif /* CONFIG_UKNETDEVTEST_RX_INTR */

#ifdef CONFIG_UKNETDEVTEST_DESCADD
	netdev_test_add_recv_desc_append(0);
	uk_pr_info("Semaphore after desc 1 %ld\n", sem_flag.count);
	netdev_test_add_recv_desc_append(1);
	uk_pr_info("Semaphore after desc 1 %ld\n", sem_flag.count);
#endif /* CONFIG_UKNETDEVTEST_DESCADD */

	netdev_receive_prepare(1);
	return 0;
}
TESTSUITE_REGISTER(netdev, test_netdev);
