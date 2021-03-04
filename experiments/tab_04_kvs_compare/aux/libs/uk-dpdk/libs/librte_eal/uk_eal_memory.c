#include <sys/mman.h>
#include <rte_memory.h>
#include <eal_internal_cfg.h>
#include <eal_memcfg.h>
#include <string.h>
#include <uk/print.h>

/**
 * Function to configure the
 * config->mem_config.
 * 1) Need to map the memory zone to in the physical segment of the array.
 */
int rte_eal_hugepage_init(void)
{
	/* for debug purposes, hugetlbfs can be disabled */
	if (internal_config.no_hugetlbfs) {
		struct rte_memseg_list *msl;
		struct rte_fbarray *arr;
		struct rte_memseg *ms;
		uint64_t page_sz;
		int n_segs, cur_seg;
		void *addr;
		struct rte_mem_config *mcfg =
			rte_eal_get_configuration()->mem_config;

		/* create a memseg list */
		msl = &mcfg->memsegs[0];

		page_sz = RTE_PGSIZE_4K;
		n_segs = internal_config.memory / page_sz;
		printf("%s: Configure %d of segments\n", __func__, n_segs);

		if (rte_fbarray_init(&msl->memseg_arr, "nohugemem", n_segs,
			sizeof(struct rte_memseg))) {
			uk_pr_err("Failed to create init_array\n");
			RTE_LOG(ERR, EAL, "Cannot allocate memseg list\n");
			return -1;
		}

		addr = mmap(NULL, internal_config.memory,
			    PROT_READ | PROT_WRITE,
			    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		if (addr == MAP_FAILED) {
			uk_pr_err("Failed to map memory\n");
			RTE_LOG(ERR, EAL, "%s: mmap() failed: %s\n", __func__,
				strerror(errno));
			return -1;
		}
		msl->base_va = addr;
		msl->page_sz = page_sz;
		msl->len = internal_config.memory;
		msl->socket_id = 0;

		/* populate memsegs. each memseg is 1 page long */
		for (cur_seg = 0; cur_seg < n_segs; cur_seg++) {
			arr = &msl->memseg_arr;

			ms = rte_fbarray_get(arr, cur_seg);
			if (rte_eal_iova_mode() == RTE_IOVA_VA)
				ms->iova = (uintptr_t)addr;
			else
				ms->iova = RTE_BAD_IOVA;
			ms->addr = addr;
			ms->hugepage_sz = page_sz;
			ms->len = page_sz;
			ms->socket_id = 0;

			rte_fbarray_set_used(arr, cur_seg);

			addr = RTE_PTR_ADD(addr, page_sz);
		}
		return 0;
	}
	return -1;
}

int rte_eal_memseg_init(void)
{
	if (internal_config.no_hugetlbfs)
		return 0;

	return -1;
}

int rte_eal_hugepage_attach(void)
{
	return -1;
}

phys_addr_t rte_mem_virt2phy(const void *virt)
{
	return ukplat_virt_to_phys(virt);
}

rte_iova_t rte_mem_virt2iova(const void *virt)
{
	return virt;
}

int rte_eal_using_phys_addrs(void)
{
	return 1;
}
