/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2010-2016 Intel Corporation
 */

#include <stdio.h>
#include <string.h>

#include <rte_errno.h>
#include <rte_ring.h>
#include <rte_mempool.h>
#include <rte_mbuf.h>
#include <uk/netbuf.h>
#include <uk/refcount.h>

static uint64_t enqueue_cnt = 0, dequeue_cnt = 0;
static uint64_t enqueue_fail_cnt = 0, dequeue_fail_cnt = 0, free_desc_cnt;
static struct rte_ring *uk_ring;

static int
uk_ring_enqueue(struct rte_mempool *mp, void * const *obj_table,
		unsigned n)
{
	int rc;
	UK_ASSERT(mp->pool_data == uk_ring);
	rc = rte_ring_sp_enqueue_bulk(mp->pool_data, obj_table, n, NULL);
	if (rc == 0) {
		return -ENOBUFS;
	}

	return 0;
}

static int
uk_ring_dequeue(struct rte_mempool *mp, void **obj_table, unsigned n)
{
	int rc = 0;

	UK_ASSERT(mp->pool_data == uk_ring);
	rc = rte_ring_sc_dequeue_bulk(mp->pool_data,
			obj_table, n, NULL);
	if (rc == 0) {
		return -ENOBUFS;
	}
	return 0;
}

static unsigned
uk_ring_get_count(const struct rte_mempool *mp)
{
	return rte_ring_count(mp->pool_data);
}

int uk_netbuf_mbuf_dtor(struct uk_netbuf *nb)
{
        struct rte_mbuf *mbuf;
	struct rte_mempool *pool;
	struct rte_pktmbuf_pool_private *user_mbp_priv;

	mbuf = (struct rte_mbuf *)nb->priv;
	pool = mbuf->pool;

#if 0
	user_mbp_priv = rte_mempool_get_priv(pool);
#endif
	uk_refcount_init(&nb->refcount, 1);
#if 0
	uk_netbuf_init_indir(nb, mbuf->buf_addr, mbuf->buf_len, RTE_PKTMBUF_HEADROOM,
			     nb->priv, uk_netbuf_mbuf_dtor);
	free_desc_cnt++;
#endif

	/* Reinit the netbuf structure */
	rte_pktmbuf_free(mbuf);
	//rte_mempool_put(pool, mbuf);
#if 0
	printf("mempool: %prte_mempool_size: %d\n", pool,
		rte_mempool_avail_count(pool));
#endif

	return 1;
}

int uk_netbuf_pool_dtor(struct uk_netbuf **nb, int cnt)
{
        struct rte_mbuf *mbuf[CONFIG_LIBUKNETDEV_MAX_PKT_BURST];
	struct rte_mempool *pool;
	struct rte_pktmbuf_pool_private *user_mbp_priv;
	int i = 0;

	for (i = 0; i < cnt; i++) {
		uk_refcount_init(&nb[i]->refcount, 1);
		mbuf[i] = (struct rte_mbuf *)nb[i]->priv;
		//rte_pktmbuf_free(mbuf[i]);
	}
	pool = mbuf[0]->pool;
#if 0
	uk_netbuf_init_indir(nb, mbuf->buf_addr, mbuf->buf_len, RTE_PKTMBUF_HEADROOM,
			     nb->priv, uk_netbuf_mbuf_dtor);
	free_desc_cnt++;
#endif

	/* Reinit the netbuf structure */
	//rte_pktmbuf_free(mbuf);
	rte_mempool_put_bulk(pool, mbuf, cnt);
#if 0
	printf("mempool: %prte_mempool_size: %d\n", pool,
		rte_mempool_avail_count(pool));
#endif

	return 1;
}

static void uk_mempool_add_elem(struct rte_mempool *mp, __rte_unused void *opaque,
                 void *obj, rte_iova_t iova)
{
        struct rte_mempool_objhdr *hdr;
        struct rte_mempool_objtlr *tlr __rte_unused;

        /* set mempool ptr in header */
        hdr = RTE_PTR_SUB(obj, mp->header_size);
        hdr->mp = mp;
        hdr->iova = iova;
        STAILQ_INSERT_TAIL(&mp->elt_list, hdr, next);
        mp->populated_size++;

        //printf("%s: populated_size: %d\n", __func__, mp->populated_size);

#ifdef RTE_LIBRTE_MEMPOOL_DEBUG
        hdr->cookie = RTE_MEMPOOL_HEADER_COOKIE2;
        tlr = __mempool_get_trailer(obj);
        tlr->cookie = RTE_MEMPOOL_TRAILER_COOKIE;
#endif
}

int uk_ring_mempool_populate(struct rte_mempool *mp,
			     unsigned int max_objs,
			     void *vaddr, rte_iova_t iova, size_t len,
			     rte_mempool_populate_obj_cb_t *obj_cb,
			     void *obj_cb_arg)
{
	size_t total_elt_sz;
	size_t off;
	unsigned int i, private_size;
	void *obj;
	int header_len = 20;
	static int header_append = 0;
	struct uk_netbuf *nb;
	void *pkt_metadata,  *mbuf;
	struct rte_mbuf *m;
	uint64_t iova_addr;
	struct rte_pktmbuf_pool_private *user_mbp_priv;

	UK_ASSERT(mp);
	user_mbp_priv = rte_mempool_get_priv(mp);
	UK_ASSERT(user_mbp_priv);
	total_elt_sz = mp->header_size + mp->elt_size + mp->trailer_size +
		sizeof(struct uk_netbuf);
	private_size = user_mbp_priv->mbuf_priv_size;

	for (off = 0, i = 0; off + total_elt_sz <= len && i < max_objs; i++) {
		pkt_metadata = vaddr +off;
		off += mp->header_size;
		obj = (char *)vaddr + off;
		nb = uk_netbuf_prepare_buf(obj,
				      total_elt_sz - mp->header_size,
				      RTE_PKTMBUF_HEADROOM,
				      sizeof(struct rte_mbuf) + private_size,
				      uk_netbuf_mbuf_dtor);
		UK_ASSERT(nb);
#if 0
		/**
		 * acquire refcount to not release memory
		 */
		uk_refcount_acquire(&nb->refcount);
#endif
		obj = (char *)vaddr + off;
		off += sizeof(struct uk_netbuf);
		mbuf = (char *)obj + sizeof(struct uk_netbuf);
		iova_addr = iova + off;
		uk_mempool_add_elem(mp, obj_cb_arg, obj,
			(iova == RTE_BAD_IOVA) ? RTE_BAD_IOVA : (iova + off));
		obj = (char *)vaddr + off;
		rte_mempool_ops_enqueue_bulk(mp, &obj, 1);
		off += mp->elt_size + mp->trailer_size;

#if 0
		printf("%s: header: %p, netbuf: %p, mbuf: %p mbuf_iova: %08x iova: %08x vaddr: %p\n",
		       __func__,pkt_metadata, nb, mbuf, iova_addr, iova, vaddr);
#endif
	}

	/**
	 * TODO:
	 * Find a better way to append the netbuf header
	 */
	if (mp->header_size < sizeof(struct uk_netbuf)) {
		mp->header_size += sizeof(struct uk_netbuf);
		header_append++;
	}

	uk_pr_info("Appending netbuf to header:%d \n", mp->header_size);
	uk_pr_info("Creating ring:%d \n", i);

	return i;
}

static int
uk_ring_alloc(struct rte_mempool *mp)
{
	int rg_flags = 0, ret;
	char rg_name[RTE_RING_NAMESIZE];
	struct rte_ring *r;

	ret = snprintf(rg_name, sizeof(rg_name),
		RTE_MEMPOOL_MZ_FORMAT, mp->name);
	if (ret < 0 || ret >= (int)sizeof(rg_name)) {
		rte_errno = ENAMETOOLONG;
		return -rte_errno;
	}

	/* ring flags */
	rg_flags |= RING_F_SP_ENQ;
	rg_flags |= RING_F_SC_DEQ;

	printf("In function %s to create %s ring of size %d\n", __func__, rg_name,
		rte_align32pow2(mp->size + 1));
	/*
	 * Allocate the ring that will be used to store objects.
	 * Ring functions will return appropriate errors if we are
	 * running as a secondary process etc., so no checks made
	 * in this function for that condition.
	 */
	r = rte_ring_create(rg_name, rte_align32pow2(mp->size + 1),
		mp->socket_id, rg_flags);
	if (r == NULL)
		return -rte_errno;
	//printf("Created a ring\n");

	mp->pool_data = r;
	uk_ring = r;

	return 0;
}

static void
uk_ring_free(struct rte_mempool *mp)
{
	rte_ring_free(mp->pool_data);
}

static ssize_t
uk_ring_calc_mem_size(const struct rte_mempool *mp,
		      uint32_t obj_num, uint32_t pg_shift,
		      size_t *min_chunk_size, size_t *align)
{
	size_t total_elt_sz;
	size_t obj_per_page, pg_num, pg_sz;
	size_t mem_size;

	total_elt_sz = mp->header_size + mp->elt_size + mp->trailer_size;
	total_elt_sz += sizeof(struct uk_netbuf);
	if (total_elt_sz == 0) {
		mem_size = 0;
	} else if (pg_shift == 0) {
		mem_size = total_elt_sz * obj_num;
	} else {
		pg_sz = (size_t)1 << pg_shift;
		obj_per_page = pg_sz / total_elt_sz;
		if (obj_per_page == 0) {
			/*
			 * Note that if object size is bigger than page size,
			 * then it is assumed that pages are grouped in subsets
			 * of physically continuous pages big enough to store
			 * at least one object.
			 */
			mem_size = RTE_ALIGN_CEIL(total_elt_sz, pg_sz) * obj_num;
		} else {
			pg_num = (obj_num + obj_per_page - 1) / obj_per_page;
			mem_size = pg_num << pg_shift;
		}
	}

	*min_chunk_size = RTE_MAX((size_t)1 << pg_shift, total_elt_sz);

	*align = RTE_MAX((size_t)RTE_CACHE_LINE_SIZE, (size_t)1 << pg_shift);
	return mem_size;
}

/*
 * The following 4 declarations of mempool ops structs address
 * the need for the backward compatible mempool handlers for
 * single/multi producers and single/multi consumers as dictated by the
 * flags provided to the rte_mempool_create function
 */
static const struct rte_mempool_ops pmd_uk_netbuf = {
	.name = "ring_uk_netbuf",
	.alloc = uk_ring_alloc,
	.free = uk_ring_free,
	.enqueue = uk_ring_enqueue,
	.dequeue = uk_ring_dequeue,
	.get_count = uk_ring_get_count,
	.populate = uk_ring_mempool_populate,
	.calc_mem_size = uk_ring_calc_mem_size,
};
MEMPOOL_REGISTER_OPS(pmd_uk_netbuf);

