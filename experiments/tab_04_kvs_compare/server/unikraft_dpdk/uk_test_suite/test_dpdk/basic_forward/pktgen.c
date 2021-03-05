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
#include <uk/netbuf.h>
#include "measure.h"
#include "pktgen.h"

#define IPPROTO_UDP    17		/* User Datagram Protocol	*/
#define IP_DEFTTL  64   /* from RFC 1340. */
#define IP_VERSION 0x40
#define IP_HDRLEN  0x05
#define IP_VHL_DEF (IP_VERSION | IP_HDRLEN)

#define RTE_MAX_SEGS_PER_PKT 1
#define TIMESTAMP_SIZE	8

uint16_t tx_pkt_seg_lengths[RTE_MAX_SEGS_PER_PKT] = {
#ifdef CONFIG_PKT_TIMESTAMP
		TXONLY_DEF_PACKET_LEN + TIMESTAMP_SIZE,
#else
	        TXONLY_DEF_PACKET_LEN,
#endif /* CONFIG_PKT_TIMESTAMP */
};

static struct rte_udp_hdr pkt_udp_hdr;
#ifdef CONFIG_PKT_TIMESTAMP
static uint16_t tx_pkt_length = TXONLY_DEF_PACKET_LEN + TIMESTAMP_SIZE;
#else
static uint16_t tx_pkt_length = TXONLY_DEF_PACKET_LEN;
#endif /* CONFIG_PKT_TIMESTAMP */
static struct rte_ipv4_hdr pkt_ip_hdr = { };
struct uk_pkt_measure pkt_stats = { };
static struct rte_ipv4_hdr pkt_ip_hdr; /**< IP header of transmitted packets. */
static struct rte_udp_hdr pkt_udp_hdr; /**< UDP header of tx packets. */



static void
copy_buf_to_pkt_segs(void* buf, unsigned len, struct rte_mbuf *pkt,
		     unsigned offset)
{
	struct rte_mbuf *seg;
	void *seg_buf;
	unsigned copy_len;

	seg = pkt;
	while (offset >= seg->data_len) {
		offset -= seg->data_len;
		seg = seg->next;
	}
	copy_len = seg->data_len - offset;
	seg_buf = rte_pktmbuf_mtod_offset(seg, char *, offset);
	while (len > copy_len) {
		rte_memcpy(seg_buf, buf, (size_t) copy_len);
		len -= copy_len;
		buf = ((char*) buf + copy_len);
		seg = seg->next;
		seg_buf = rte_pktmbuf_mtod(seg, char *);
		copy_len = seg->data_len;
	}
	rte_memcpy(seg_buf, buf, (size_t) len);
}

static inline void
copy_buf_to_pkt(void* buf, unsigned len, struct rte_mbuf *pkt, unsigned offset)
{
	if (offset + len <= pkt->data_len) {
		rte_memcpy(rte_pktmbuf_mtod_offset(pkt, char *, offset),
			buf, (size_t) len);
		return;
	}
	copy_buf_to_pkt_segs(buf, len, pkt, offset);
}

static void
setup_pkt_udp_ip_headers(struct rte_ipv4_hdr *ip_hdr,
			 struct rte_udp_hdr *udp_hdr,
			 uint16_t pkt_data_len, 
			 uint32_t src_ipv4_addr, uint32_t dst_ipv4_addr,
			 uint16_t src_port, uint16_t dst_port)
{
	uint16_t *ptr16;
	uint32_t ip_cksum;
	uint16_t pkt_len;

	/*
	 * Initialize UDP header.
	 */
	pkt_len = (uint16_t) (pkt_data_len + sizeof(struct rte_udp_hdr));
	udp_hdr->src_port = rte_cpu_to_be_16(src_port);
	udp_hdr->dst_port = rte_cpu_to_be_16(dst_port);
	udp_hdr->dgram_len      = RTE_CPU_TO_BE_16(pkt_len);
	udp_hdr->dgram_cksum    = 0; /* No UDP checksum. */

	/*
	 * Initialize IP header.
	 */
	pkt_len = (uint16_t) (pkt_len + sizeof(struct rte_ipv4_hdr));
	ip_hdr->version_ihl   = IP_VHL_DEF;
	ip_hdr->type_of_service   = 0;
	ip_hdr->fragment_offset = 0;
	ip_hdr->time_to_live   = IP_DEFTTL;
	ip_hdr->next_proto_id = IPPROTO_UDP;
	ip_hdr->packet_id = 0;
	ip_hdr->total_length   = RTE_CPU_TO_BE_16(pkt_len);
	ip_hdr->src_addr = rte_cpu_to_be_32(src_ipv4_addr);
	ip_hdr->dst_addr = rte_cpu_to_be_32(dst_ipv4_addr);

	/*
	 * Compute IP header checksum.
	 */
	ptr16 = (unaligned_uint16_t*) ip_hdr;
	ip_cksum = 0;
	ip_cksum += ptr16[0]; ip_cksum += ptr16[1];
	ip_cksum += ptr16[2]; ip_cksum += ptr16[3];
	ip_cksum += ptr16[4];
	ip_cksum += ptr16[6]; ip_cksum += ptr16[7];
	ip_cksum += ptr16[8]; ip_cksum += ptr16[9];

	/*
	 * Reduce 32 bit checksum to 16 bits and complement it.
	 */
	ip_cksum = ((ip_cksum & 0xFFFF0000) >> 16) +
		(ip_cksum & 0x0000FFFF);
	if (ip_cksum > 65535)
		ip_cksum -= 65535;
	ip_cksum = (~ip_cksum) & 0x0000FFFF;
	if (ip_cksum == 0)
		ip_cksum = 0xFFFF;
	ip_hdr->hdr_checksum = (uint16_t) ip_cksum;
}

static size_t _genudp_dgram(struct rte_mbuf *pkt, 
			    const struct rte_ether_addr *src_mac,
			    const struct rte_ether_addr *dst_mac,
			    uint32_t src_ipv4_addr, uint32_t dst_ipv4_addr,
			    uint16_t src_port, uint16_t dst_port,
			    uint8_t ttl, uint64_t ol_flags, uint64_t rdtsc)
{
	unsigned i;
	unsigned full_hdr_len;
	uint16_t udp_data_len;
	static char c = 'a';
	size_t pkt_len;
	struct rte_mbuf *pkt_seg;
	uint16_t offset = 0;
	uint16_t *ptr16;
	uint32_t ip_cksum;

	/* Initialize the mbuf structure */
	rte_pktmbuf_reset_headroom(pkt);
	pkt->data_len = tx_pkt_seg_lengths[0];
	pkt->ol_flags = ol_flags;
	//pkt->vlan_tci = vlan_tci;
	//pkt->vlan_tci_outer = vlan_tci_outer;
	pkt->l2_len = sizeof(struct rte_ether_hdr);
	pkt->l3_len = sizeof(struct rte_ipv4_hdr);

	pkt_len = pkt->data_len;
	pkt_seg = pkt;
	pkt_seg->next = NULL; /* Last segment of packet. */

	struct rte_ether_hdr *eth_hdr = rte_pktmbuf_mtod_offset(pkt, char *, offset);
	offset = sizeof(*eth_hdr);
	struct rte_ipv4_hdr *ipv4_hdr = rte_pktmbuf_mtod_offset(pkt, char *, offset);
	offset += sizeof(*ipv4_hdr);
	struct rte_udp_hdr *udp_hdr = rte_pktmbuf_mtod_offset(pkt, char *, offset);


	full_hdr_len = sizeof(*eth_hdr) + sizeof(*ipv4_hdr) + sizeof(*udp_hdr);
	if (pkt_len < sizeof(*eth_hdr))
		return 0;		/* buffer is too small -> abort */
	/* max packet size */
	if (pkt_len - sizeof(*eth_hdr) > UINT16_MAX)
		pkt_len = (size_t) UINT16_MAX + sizeof(*eth_hdr);

	/* header fields */
	rte_ether_addr_copy(src_mac, &eth_hdr->s_addr);
	rte_ether_addr_copy(dst_mac, &eth_hdr->d_addr);
	eth_hdr->ether_type = rte_cpu_to_be_16(RTE_ETHER_TYPE_IPV4);
	if (pkt_len < (sizeof(*eth_hdr) + sizeof(*ipv4_hdr)))
		return sizeof(*eth_hdr); /* Ethernet header only */

	ipv4_hdr->src_addr = rte_cpu_to_be_32(src_ipv4_addr);
	ipv4_hdr->dst_addr = rte_cpu_to_be_32(dst_ipv4_addr);
	ipv4_hdr->packet_id = rte_cpu_to_be_16(0);
	ipv4_hdr->fragment_offset = rte_cpu_to_be_16(0);
	ipv4_hdr->type_of_service = 0;
	ipv4_hdr->version_ihl = IP_VHL_DEF;
	ipv4_hdr->total_length = rte_cpu_to_be_16((uint16_t) (pkt_len - sizeof(*eth_hdr)));
	ipv4_hdr->time_to_live = ttl;

	/* UDP IPv4 packet */
	ipv4_hdr->next_proto_id = IPPROTO_UDP;
	/*
	 * Compute IP header checksum.
	 */
	ptr16 = (uint16_t*) ipv4_hdr;
	ip_cksum = 0;
	ip_cksum += ptr16[0]; ip_cksum += ptr16[1];
	ip_cksum += ptr16[2]; ip_cksum += ptr16[3];
	ip_cksum += ptr16[4];
	ip_cksum += ptr16[6]; ip_cksum += ptr16[7];
	ip_cksum += ptr16[8]; ip_cksum += ptr16[9];
	/*
	 * Reduce 32 bit checksum to 16 bits and complement it.
	 */
	ip_cksum = ((ip_cksum & 0xFFFF0000) >> 16) +
	    (ip_cksum & 0x0000FFFF);
	if (ip_cksum > 65535)
	    ip_cksum -= 65535;
	ip_cksum = (~ip_cksum) & 0x0000FFFF;
	if (ip_cksum == 0)
	    ip_cksum = 0xFFFF;
	ipv4_hdr->hdr_checksum = (uint16_t) ip_cksum;

	//ipv4_hdr->hdr_checksum = _gen_ipv4_hdr_checksum_be_16(ipv4_hdr);
	udp_hdr->src_port = rte_cpu_to_be_16(src_port);
	udp_hdr->dst_port = rte_cpu_to_be_16(dst_port);
	udp_hdr->dgram_len = rte_cpu_to_be_16(rte_be_to_cpu_16(ipv4_hdr->total_length) - sizeof(*ipv4_hdr));
	udp_hdr->dgram_cksum = 0; /* no UDP checksum */
	udp_data_len = rte_be_to_cpu_16(udp_hdr->dgram_len) - sizeof(*udp_hdr);
	/* UDP data area */
	char *udp_data = rte_pktmbuf_mtod_offset(pkt, char *, full_hdr_len); 
	*((uint64_t *)udp_data) = rdtsc;
	for (i = sizeof(rdtsc); i < udp_data_len; i++)
		*((char *) udp_data + i) = c++;

	pkt->pkt_len = pkt_len;

	return 0;
}

#ifdef CONFIG_USE_DPDK_PMD
bool pkt_burst_prepare(struct rte_mbuf *pkt, struct rte_mempool *mbp,
		       struct rte_ether_hdr *eth_hdr, const uint64_t ol_flags,
		       uint64_t timestamp)
{
	struct rte_mbuf *pkt_segs[RTE_MAX_SEGS_PER_PKT];
	struct rte_mbuf *pkt_seg;
	uint32_t nb_segs, pkt_len;
	uint8_t i;
	nb_segs = 1;

#if 0

	if (nb_segs > 1) {
		if (rte_mempool_get_bulk(mbp, (void **)pkt_segs, nb_segs - 1))
			return false;
	}
#endif

	rte_pktmbuf_reset_headroom(pkt);
	pkt->data_len = tx_pkt_seg_lengths[0];
	pkt->ol_flags = ol_flags;
	//pkt->vlan_tci = vlan_tci;
	//pkt->vlan_tci_outer = vlan_tci_outer;
	pkt->l2_len = sizeof(struct rte_ether_hdr);
	pkt->l3_len = sizeof(struct rte_ipv4_hdr);

	pkt_len = pkt->data_len;
	pkt_seg = pkt;
#if 0
	for (i = 1; i < nb_segs; i++) {
		pkt_seg->next = pkt_segs[i - 1];
		pkt_seg = pkt_seg->next;
		pkt_seg->data_len = tx_pkt_seg_lengths[i];
		pkt_len += pkt_seg->data_len;
	}
#endif
	pkt_seg->next = NULL; /* Last segment of packet. */
	/*
	 * Copy headers in first packet segment(s).
	 */
	copy_buf_to_pkt(eth_hdr, sizeof(*eth_hdr), pkt, 0);
	copy_buf_to_pkt(&pkt_ip_hdr, sizeof(pkt_ip_hdr), pkt,
			sizeof(struct rte_ether_hdr));
	//if (txonly_multi_flow) {
	//	struct rte_ipv4_hdr *ip_hdr;
	//	uint32_t addr;

	//	ip_hdr = rte_pktmbuf_mtod_offset(pkt,
	//			struct rte_ipv4_hdr *,
	//			sizeof(struct rte_ether_hdr));
	//	/*
	//	 * Generate multiple flows by varying IP src addr. This
	//	 * enables packets are well distributed by RSS in
	//	 * receiver side if any and txonly mode can be a decent
	//	 * packet generator for developer's quick performance
	//	 * regression test.
	//	 */
	//	addr = (tx_ip_dst_addr | (ip_var++ << 8)) + rte_lcore_id();
	//	ip_hdr->src_addr = rte_cpu_to_be_32(addr);
	//}
	copy_buf_to_pkt(&pkt_udp_hdr, sizeof(pkt_udp_hdr), pkt,
			sizeof(struct rte_ether_hdr) +
			sizeof(struct rte_ipv4_hdr));
#ifdef CONFIG_PKT_TIMESTAMP
	copy_buf_to_pkt(&timestamp, sizeof(timestamp), pkt,
			sizeof(struct rte_ether_hdr) +
			sizeof(struct rte_ipv4_hdr) +
			sizeof(struct rte_udp_hdr));
#endif /* CONFIG_PKT_TIMESTAMP */
	/*
	 * Complete first mbuf of packet and append it to the
	 * burst of packets to be transmitted.
	 */
	pkt->nb_segs = nb_segs;
	pkt->pkt_len = pkt_len;


	return true;
}
#else

bool pkt_burst_prepare(struct rte_mbuf *pkt, struct rte_mempool *mbp,
		       struct rte_ether_hdr *eth_hdr, const uint64_t ol_flags,
		       uint64_t timestamp, struct uk_netbuf **snd_buf)
{
	struct rte_mbuf *pkt_segs[RTE_MAX_SEGS_PER_PKT];
	struct rte_mbuf *pkt_seg;
	uint32_t nb_segs, pkt_len;
	uint8_t i;
	nb_segs = 1;

#if 0

	if (nb_segs > 1) {
		if (rte_mempool_get_bulk(mbp, (void **)pkt_segs, nb_segs - 1))
			return false;
	}
#endif

	rte_pktmbuf_reset_headroom(pkt);
	pkt->data_len = tx_pkt_seg_lengths[0];
	pkt->ol_flags = ol_flags;
	//pkt->vlan_tci = vlan_tci;
	//pkt->vlan_tci_outer = vlan_tci_outer;
	pkt->l2_len = sizeof(struct rte_ether_hdr);
	pkt->l3_len = sizeof(struct rte_ipv4_hdr);

	pkt_len = pkt->data_len;
	pkt_seg = pkt;
#if 0
	for (i = 1; i < nb_segs; i++) {
		pkt_seg->next = pkt_segs[i - 1];
		pkt_seg = pkt_seg->next;
		pkt_seg->data_len = tx_pkt_seg_lengths[i];
		pkt_len += pkt_seg->data_len;
	}
#endif
	pkt_seg->next = NULL; /* Last segment of packet. */
	/*
	 * Copy headers in first packet segment(s).
	 */
	copy_buf_to_pkt(eth_hdr, sizeof(*eth_hdr), pkt, 0);
	copy_buf_to_pkt(&pkt_ip_hdr, sizeof(pkt_ip_hdr), pkt,
			sizeof(struct rte_ether_hdr));
	//if (txonly_multi_flow) {
	//	struct rte_ipv4_hdr *ip_hdr;
	//	uint32_t addr;

	//	ip_hdr = rte_pktmbuf_mtod_offset(pkt,
	//			struct rte_ipv4_hdr *,
	//			sizeof(struct rte_ether_hdr));
	//	/*
	//	 * Generate multiple flows by varying IP src addr. This
	//	 * enables packets are well distributed by RSS in
	//	 * receiver side if any and txonly mode can be a decent
	//	 * packet generator for developer's quick performance
	//	 * regression test.
	//	 */
	//	addr = (tx_ip_dst_addr | (ip_var++ << 8)) + rte_lcore_id();
	//	ip_hdr->src_addr = rte_cpu_to_be_32(addr);
	//}
	copy_buf_to_pkt(&pkt_udp_hdr, sizeof(pkt_udp_hdr), pkt,
			sizeof(struct rte_ether_hdr) +
			sizeof(struct rte_ipv4_hdr));
#ifdef CONFIG_PKT_TIMESTAMP
	copy_buf_to_pkt(&timestamp, sizeof(timestamp), pkt,
			sizeof(struct rte_ether_hdr) +
			sizeof(struct rte_ipv4_hdr) +
			sizeof(struct rte_udp_hdr));
#endif /* CONFIG_PKT_TIMESTAMP */
	/*
	 * Complete first mbuf of packet and append it to the
	 * burst of packets to be transmitted.
	 */
	pkt->nb_segs = nb_segs;
	pkt->pkt_len = pkt_len;

	//struct uk_netbuf *nb = (struct uk_netbuf *)((void *)((uintptr_t) pkt) - sizeof(struct uk_netbuf));
	struct uk_netbuf *nb = (struct uk_netbuf *)pkt->userdata;
	UK_ASSERT(nb);
	nb->len = pkt_len;
	*snd_buf = nb;

	return true;
}
#endif /* !CONFIG_USE_DPDK_PMD */

void tx_pkt_setup(uint32_t src_ipv4_addr, uint32_t dst_ipv4_addr,
		  uint16_t src_port, uint16_t dst_port)
{
	uint16_t pkt_data_len;

	pkt_data_len = (uint16_t) (tx_pkt_length - (
					sizeof(struct rte_ether_hdr) +
					sizeof(struct rte_ipv4_hdr) +
					sizeof(struct rte_udp_hdr)));
	setup_pkt_udp_ip_headers(&pkt_ip_hdr, &pkt_udp_hdr, pkt_data_len,
				 src_ipv4_addr, dst_ipv4_addr, src_port, dst_port);
}

#ifdef CONFIG_USE_DPDK_PMD
bool pkt_burst_prepare_data(struct rte_mbuf *pkt, struct rte_mempool *mbp,
		       struct rte_ether_hdr *eth_hdr, const uint64_t ol_flags,
		       uint64_t timestamp, void *data, uint16_t size)
{
	struct rte_mbuf *pkt_segs[RTE_MAX_SEGS_PER_PKT];
	struct rte_mbuf *pkt_seg;
	uint32_t nb_segs, pkt_len;
	uint8_t i;
	nb_segs = 1;

	UK_ASSERT(data);
#if 0

	if (nb_segs > 1) {
		if (rte_mempool_get_bulk(mbp, (void **)pkt_segs, nb_segs - 1))
			return false;
	}
#endif

	rte_pktmbuf_reset_headroom(pkt);
	pkt->data_len = tx_pkt_seg_lengths[0] + size;
	pkt->ol_flags = ol_flags;
	//pkt->vlan_tci = vlan_tci;
	//pkt->vlan_tci_outer = vlan_tci_outer;
	pkt->l2_len = sizeof(struct rte_ether_hdr);
	pkt->l3_len = sizeof(struct rte_ipv4_hdr);

	pkt_len = pkt->data_len;
	pkt_seg = pkt;
#if 0
	for (i = 1; i < nb_segs; i++) {
		pkt_seg->next = pkt_segs[i - 1];
		pkt_seg = pkt_seg->next;
		pkt_seg->data_len = tx_pkt_seg_lengths[i];
		pkt_len += pkt_seg->data_len;
	}
#endif
	pkt_seg->next = NULL; /* Last segment of packet. */
	/*
	 * Copy headers in first packet segment(s).
	 */
	copy_buf_to_pkt(eth_hdr, sizeof(*eth_hdr), pkt, 0);
	copy_buf_to_pkt(&pkt_ip_hdr, sizeof(pkt_ip_hdr), pkt,
			sizeof(struct rte_ether_hdr));
	//if (txonly_multi_flow) {
	//	struct rte_ipv4_hdr *ip_hdr;
	//	uint32_t addr;

	//	ip_hdr = rte_pktmbuf_mtod_offset(pkt,
	//			struct rte_ipv4_hdr *,
	//			sizeof(struct rte_ether_hdr));
	//	/*
	//	 * Generate multiple flows by varying IP src addr. This
	//	 * enables packets are well distributed by RSS in
	//	 * receiver side if any and txonly mode can be a decent
	//	 * packet generator for developer's quick performance
	//	 * regression test.
	//	 */
	//	addr = (tx_ip_dst_addr | (ip_var++ << 8)) + rte_lcore_id();
	//	ip_hdr->src_addr = rte_cpu_to_be_32(addr);
	//}
	copy_buf_to_pkt(&pkt_udp_hdr, sizeof(pkt_udp_hdr), pkt,
			sizeof(struct rte_ether_hdr) +
			sizeof(struct rte_ipv4_hdr));
	copy_buf_to_pkt(data, size, pkt,
			sizeof(struct rte_ether_hdr) +
			sizeof(struct rte_ipv4_hdr) +
			sizeof(struct rte_udp_hdr));
	/*
	 * Complete first mbuf of packet and append it to the
	 * burst of packets to be transmitted.
	 */
	pkt->nb_segs = nb_segs;
	pkt->pkt_len = pkt_len;

	return true;
}
#else
bool pkt_burst_prepare_data(struct rte_mbuf *pkt, struct rte_mempool *mbp,
		       struct rte_ether_hdr *eth_hdr, const uint64_t ol_flags,
		       uint64_t timestamp, struct uk_netbuf **snd_buf,
		       void *data, uint16_t size)
{
	struct rte_mbuf *pkt_segs[RTE_MAX_SEGS_PER_PKT];
	struct rte_mbuf *pkt_seg;
	uint32_t nb_segs, pkt_len;
	uint8_t i;
	nb_segs = 1;

#if 0

	if (nb_segs > 1) {
		if (rte_mempool_get_bulk(mbp, (void **)pkt_segs, nb_segs - 1))
			return false;
	}
#endif

	rte_pktmbuf_reset_headroom(pkt);
	pkt->data_len = tx_pkt_seg_lengths[0] + size;
	pkt->ol_flags = ol_flags;
	//pkt->vlan_tci = vlan_tci;
	//pkt->vlan_tci_outer = vlan_tci_outer;
	pkt->l2_len = sizeof(struct rte_ether_hdr);
	pkt->l3_len = sizeof(struct rte_ipv4_hdr);

	pkt_len = pkt->data_len;
	pkt_seg = pkt;
#if 0
	for (i = 1; i < nb_segs; i++) {
		pkt_seg->next = pkt_segs[i - 1];
		pkt_seg = pkt_seg->next;
		pkt_seg->data_len = tx_pkt_seg_lengths[i];
		pkt_len += pkt_seg->data_len;
	}
#endif
	pkt_seg->next = NULL; /* Last segment of packet. */
	/*
	 * Copy headers in first packet segment(s).
	 */
	copy_buf_to_pkt(eth_hdr, sizeof(*eth_hdr), pkt, 0);
	copy_buf_to_pkt(&pkt_ip_hdr, sizeof(pkt_ip_hdr), pkt,
			sizeof(struct rte_ether_hdr));
	//if (txonly_multi_flow) {
	//	struct rte_ipv4_hdr *ip_hdr;
	//	uint32_t addr;

	//	ip_hdr = rte_pktmbuf_mtod_offset(pkt,
	//			struct rte_ipv4_hdr *,
	//			sizeof(struct rte_ether_hdr));
	//	/*
	//	 * Generate multiple flows by varying IP src addr. This
	//	 * enables packets are well distributed by RSS in
	//	 * receiver side if any and txonly mode can be a decent
	//	 * packet generator for developer's quick performance
	//	 * regression test.
	//	 */
	//	addr = (tx_ip_dst_addr | (ip_var++ << 8)) + rte_lcore_id();
	//	ip_hdr->src_addr = rte_cpu_to_be_32(addr);
	//}
	copy_buf_to_pkt(&pkt_udp_hdr, sizeof(pkt_udp_hdr), pkt,
			sizeof(struct rte_ether_hdr) +
			sizeof(struct rte_ipv4_hdr));
	copy_buf_to_pkt(data, size, pkt,
			sizeof(struct rte_ether_hdr) +
			sizeof(struct rte_ipv4_hdr) +
			sizeof(struct rte_udp_hdr));
	/*
	 * Complete first mbuf of packet and append it to the
	 * burst of packets to be transmitted.
	 */
	pkt->nb_segs = nb_segs;
	pkt->pkt_len = pkt_len;

	//struct uk_netbuf *nb = (struct uk_netbuf *)((void *)((uintptr_t) pkt) - sizeof(struct uk_netbuf));
	struct uk_netbuf *nb = (struct uk_netbuf *)pkt->userdata;
	UK_ASSERT(nb);
	nb->len = pkt_len;
	*snd_buf = nb;

	return true;
}
#endif /* !CONFIG_USE_DPDK_PMD */
