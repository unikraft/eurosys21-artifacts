/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2017-2018 Intel Corporation
 */

#include <inttypes.h>
#include <uk/essentials.h>
#include <limits.h>
#include <sys/mman.h>
#include <stdint.h>
#include <errno.h>
#include <sys/file.h>
#include <string.h>
#include <unistd.h>

#include <rte_common.h>
#include <rte_log.h>
#include <rte_errno.h>
#include <rte_spinlock.h>
#include <rte_tailq.h>

#include "eal_filesystem.h"
#include "eal_private.h"

#include "rte_fbarray.h"

#define MASK_SHIFT 6ULL
#define MASK_ALIGN (1ULL << MASK_SHIFT)
#define MASK_LEN_TO_IDX(x) ((x) >> MASK_SHIFT)
#define MASK_LEN_TO_MOD(x) ((x) - RTE_ALIGN_FLOOR(x, MASK_ALIGN))
#define MASK_GET_IDX(idx, mod) ((idx << MASK_SHIFT) + mod)

/*
 * This is a mask that is always stored at the end of array, to provide fast
 * way of finding free/used spots without looping through each element.
 */

struct used_mask {
	unsigned int n_masks;
	uint64_t data[];
};

static size_t
calc_mask_size(unsigned int len)
{
	/* mask must be multiple of MASK_ALIGN, even though length of array
	 * itself may not be aligned on that boundary.
	 */
	len = RTE_ALIGN_CEIL(len, MASK_ALIGN);
	return sizeof(struct used_mask) +
			sizeof(uint64_t) * MASK_LEN_TO_IDX(len);
}

static size_t
calc_data_size(size_t page_sz, unsigned int elt_sz, unsigned int len)
{
	size_t data_sz = elt_sz * len;
	size_t msk_sz = calc_mask_size(len);
	return RTE_ALIGN_CEIL(data_sz + msk_sz, page_sz);
}

static struct used_mask *
get_used_mask(void *data, unsigned int elt_sz, unsigned int len)
{
	return (struct used_mask *) RTE_PTR_ADD(data, elt_sz * len);
}

static int
resize_and_map(int fd __unused, void *addr, size_t len)
{
	char path[PATH_MAX] __maybe_unused;
	void *map_addr;

#ifdef CONFIG_FTRUNCATE
	if (ftruncate(fd, len)) {
		RTE_LOG(ERR, EAL, "Cannot truncate %s\n", path);
		/* pass errno up the chain */
		rte_errno = errno;
		return -1;
	}
#endif /* CONFIG_FTRUNCATE */

	map_addr = mmap(addr, len, PROT_READ | PROT_WRITE,
			MAP_SHARED | MAP_FIXED, fd, 0);
	if (map_addr != addr) {
		RTE_LOG(ERR, EAL, "mmap() failed: %s\n", strerror(errno));
		/* pass errno up the chain */
		rte_errno = errno;
		return -1;
	}
	return 0;
}

static int
find_next_n(const struct rte_fbarray *arr, unsigned int start, unsigned int n,
	    bool used)
{
	const struct used_mask *msk = get_used_mask(arr->data, arr->elt_sz,
			arr->len);
	unsigned int msk_idx, lookahead_idx, first, first_mod;
	unsigned int last, last_mod;
	uint64_t last_msk, ignore_msk;

	/*
	 * mask only has granularity of MASK_ALIGN, but start may not be aligned
	 * on that boundary, so construct a special mask to exclude anything we
	 * don't want to see to avoid confusing ctz.
	 */
	first = MASK_LEN_TO_IDX(start);
	first_mod = MASK_LEN_TO_MOD(start);
	ignore_msk = ~((1ULL << first_mod) - 1);

	/* array length may not be aligned, so calculate ignore mask for last
	 * mask index.
	 */
	last = MASK_LEN_TO_IDX(arr->len);
	last_mod = MASK_LEN_TO_MOD(arr->len);
	last_msk = ~(-1ULL << last_mod);

	for (msk_idx = first; msk_idx < msk->n_masks; msk_idx++) {
		uint64_t cur_msk, lookahead_msk;
		unsigned int run_start, clz, left;
		bool found = false;
		/*
		 * The process of getting n consecutive bits for arbitrary n is
		 * a bit involved, but here it is in a nutshell:
		 *
		 *  1. let n be the number of consecutive bits we're looking for
		 *  2. check if n can fit in one mask, and if so, do n-1
		 *     rshift-ands to see if there is an appropriate run inside
		 *     our current mask
		 *    2a. if we found a run, bail out early
		 *    2b. if we didn't find a run, proceed
		 *  3. invert the mask and count leading zeroes (that is, count
		 *     how many consecutive set bits we had starting from the
		 *     end of current mask) as k
		 *    3a. if k is 0, continue to next mask
		 *    3b. if k is not 0, we have a potential run
		 *  4. to satisfy our requirements, next mask must have n-k
		 *     consecutive set bits right at the start, so we will do
		 *     (n-k-1) rshift-ands and check if first bit is set.
		 *
		 * Step 4 will need to be repeated if (n-k) > MASK_ALIGN until
		 * we either run out of masks, lose the run, or find what we
		 * were looking for.
		 */
		cur_msk = msk->data[msk_idx];
		left = n;

		/* if we're looking for free spaces, invert the mask */
		if (!used)
			cur_msk = ~cur_msk;

		/* combine current ignore mask with last index ignore mask */
		if (msk_idx == last)
			ignore_msk |= last_msk;

		/* if we have an ignore mask, ignore once */
		if (ignore_msk) {
			cur_msk &= ignore_msk;
			ignore_msk = 0;
		}

		/* if n can fit in within a single mask, do a search */
		if (n <= MASK_ALIGN) {
			uint64_t tmp_msk = cur_msk;
			unsigned int s_idx;
			for (s_idx = 0; s_idx < n - 1; s_idx++)
				tmp_msk &= tmp_msk >> 1ULL;
			/* we found what we were looking for */
			if (tmp_msk != 0) {
				run_start = __builtin_ctzll(tmp_msk);
				return MASK_GET_IDX(msk_idx, run_start);
			}
		}

		/*
		 * we didn't find our run within the mask, or n > MASK_ALIGN,
		 * so we're going for plan B.
		 */

		/* count leading zeroes on inverted mask */
		if (~cur_msk == 0)
			clz = sizeof(cur_msk) * 8;
		else
			clz = __builtin_clzll(~cur_msk);

		/* if there aren't any runs at the end either, just continue */
		if (clz == 0)
			continue;

		/* we have a partial run at the end, so try looking ahead */
		run_start = MASK_ALIGN - clz;
		left -= clz;

		for (lookahead_idx = msk_idx + 1; lookahead_idx < msk->n_masks;
				lookahead_idx++) {
			unsigned int s_idx, need;
			lookahead_msk = msk->data[lookahead_idx];

			/* if we're looking for free space, invert the mask */
			if (!used)
				lookahead_msk = ~lookahead_msk;

			/* figure out how many consecutive bits we need here */
			need = RTE_MIN(left, MASK_ALIGN);

			for (s_idx = 0; s_idx < need - 1; s_idx++)
				lookahead_msk &= lookahead_msk >> 1ULL;

			/* if first bit is not set, we've lost the run */
			if ((lookahead_msk & 1) == 0) {
				/*
				 * we've scanned this far, so we know there are
				 * no runs in the space we've lookahead-scanned
				 * as well, so skip that on next iteration.
				 */
				ignore_msk = ~((1ULL << need) - 1);
				msk_idx = lookahead_idx;
				break;
			}

			left -= need;

			/* check if we've found what we were looking for */
			if (left == 0) {
				found = true;
				break;
			}
		}

		/* we didn't find anything, so continue */
		if (!found)
			continue;

		return MASK_GET_IDX(msk_idx, run_start);
	}
	/* we didn't find anything */
	rte_errno = used ? -ENOENT : -ENOSPC;
	return -1;
}

static int
find_next(const struct rte_fbarray *arr, unsigned int start, bool used)
{
	const struct used_mask *msk = get_used_mask(arr->data, arr->elt_sz,
			arr->len);
	unsigned int idx, first, first_mod;
	unsigned int last, last_mod;
	uint64_t last_msk, ignore_msk;

	/*
	 * mask only has granularity of MASK_ALIGN, but start may not be aligned
	 * on that boundary, so construct a special mask to exclude anything we
	 * don't want to see to avoid confusing ctz.
	 */
	first = MASK_LEN_TO_IDX(start);
	first_mod = MASK_LEN_TO_MOD(start);
	ignore_msk = ~((1ULL << first_mod) - 1ULL);

	/* array length may not be aligned, so calculate ignore mask for last
	 * mask index.
	 */
	last = MASK_LEN_TO_IDX(arr->len);
	last_mod = MASK_LEN_TO_MOD(arr->len);
	last_msk = ~(-(1ULL) << last_mod);

	for (idx = first; idx < msk->n_masks; idx++) {
		uint64_t cur = msk->data[idx];
		int found;

		/* if we're looking for free entries, invert mask */
		if (!used)
			cur = ~cur;

		if (idx == last)
			cur &= last_msk;

		/* ignore everything before start on first iteration */
		if (idx == first)
			cur &= ignore_msk;

		/* check if we have any entries */
		if (cur == 0)
			continue;

		/*
		 * find first set bit - that will correspond to whatever it is
		 * that we're looking for.
		 */
		found = __builtin_ctzll(cur);
		return MASK_GET_IDX(idx, found);
	}
	/* we didn't find anything */
	rte_errno = used ? -ENOENT : -ENOSPC;
	return -1;
}

static int
find_contig(const struct rte_fbarray *arr, unsigned int start, bool used)
{
	const struct used_mask *msk = get_used_mask(arr->data, arr->elt_sz,
			arr->len);
	unsigned int idx, first, first_mod;
	unsigned int last, last_mod;
	uint64_t last_msk;
	unsigned int need_len, result = 0;

	/* array length may not be aligned, so calculate ignore mask for last
	 * mask index.
	 */
	last = MASK_LEN_TO_IDX(arr->len);
	last_mod = MASK_LEN_TO_MOD(arr->len);
	last_msk = ~(-(1ULL) << last_mod);

	first = MASK_LEN_TO_IDX(start);
	first_mod = MASK_LEN_TO_MOD(start);
	for (idx = first; idx < msk->n_masks; idx++, result += need_len) {
		uint64_t cur = msk->data[idx];
		unsigned int run_len;

		need_len = MASK_ALIGN;

		/* if we're looking for free entries, invert mask */
		if (!used)
			cur = ~cur;

		/* if this is last mask, ignore everything after last bit */
		if (idx == last)
			cur &= last_msk;

		/* ignore everything before start on first iteration */
		if (idx == first) {
			cur >>= first_mod;
			/* at the start, we don't need the full mask len */
			need_len -= first_mod;
		}

		/* we will be looking for zeroes, so invert the mask */
		cur = ~cur;

		/* if mask is zero, we have a complete run */
		if (cur == 0)
			continue;

		/*
		 * see if current run ends before mask end.
		 */
		run_len = __builtin_ctzll(cur);

		/* add however many zeroes we've had in the last run and quit */
		if (run_len < need_len) {
			result += run_len;
			break;
		}
	}
	return result;
}

static int
set_used(struct rte_fbarray *arr, unsigned int idx, bool used)
{
	struct used_mask *msk;
	uint64_t msk_bit = 1ULL << MASK_LEN_TO_MOD(idx);
	unsigned int msk_idx = MASK_LEN_TO_IDX(idx);
	bool already_used;
	int ret = -1;

	if (arr == NULL || idx >= arr->len) {
		rte_errno = EINVAL;
		return -1;
	}
	msk = get_used_mask(arr->data, arr->elt_sz, arr->len);
	ret = 0;

	/* prevent array from changing under us */
	rte_rwlock_write_lock(&arr->rwlock);

	already_used = (msk->data[msk_idx] & msk_bit) != 0;

	/* nothing to be done */
	if (used == already_used)
		goto out;

	if (used) {
		msk->data[msk_idx] |= msk_bit;
		arr->count++;
	} else {
		msk->data[msk_idx] &= ~msk_bit;
		arr->count--;
	}
out:
	rte_rwlock_write_unlock(&arr->rwlock);

	return ret;
}

static int
fully_validate(const char *name, unsigned int elt_sz, unsigned int len)
{
	if (name == NULL || elt_sz == 0 || len == 0 || len > INT_MAX) {
		rte_errno = EINVAL;
		return -1;
	}

	if (strnlen(name, RTE_FBARRAY_NAME_LEN) == RTE_FBARRAY_NAME_LEN) {
		rte_errno = ENAMETOOLONG;
		return -1;
	}
	return 0;
}
