#include <stdbool.h>
#include <eal_memalloc.h>

int eal_memalloc_free_seg_bulk(struct rte_memseg **ms, int n_segs)
{
	return -1;
}

struct rte_memseg *eal_memalloc_alloc_seg(size_t page_sz, int socket)
{
	return NULL;
}

int eal_memalloc_alloc_seg_bulk(struct rte_memseg **ms, int n_segs,
				size_t page_sz, int socket, bool exact)
{
	return 0;
}

int eal_memalloc_free_seg(struct rte_memseg *ms)
{
	return -1;
}

int eal_memalloc_sync_with_primary(void)
{
	return -1;
}

int eal_memalloc_get_seg_fd(int list_idx, int seg_idx)
{
	return -1;
}

int eal_memalloc_set_seg_list_fd(int list_idx, int fd)
{
	return -1;
}

int eal_memalloc_get_seg_fd_offset(int list_idx, int seg_idx, size_t *offset)
{
	return -1;
}

int eal_memalloc_init(void)
{
	return 0;
}
