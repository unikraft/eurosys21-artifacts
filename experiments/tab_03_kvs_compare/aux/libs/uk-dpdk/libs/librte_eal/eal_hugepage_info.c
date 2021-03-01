#include <eal_hugepages.h>
#include <uk/alloc.h>
#include <uk/asm/limits.h>
#include <eal_internal_cfg.h>
#include <rte_memory.h>

static uint32_t huge_page_size = RTE_PGSIZE_2M;

int eal_hugepage_info_init(void)
{
	uint32_t nr_page;
	int left_mem = 0;
	struct hugepage_info *hpi;	

	hpi = &internal_config.hugepage_info[0];

	/**
	 * Fetch the left over memory.
	 */
	left_mem = uk_alloc_availmem(uk_alloc_get_default());
	if (left_mem < 0) {
		uk_pr_err("Failed to fetch the available memory\n");
		return left_mem;
	}
	hpi->hugepage_sz =  huge_page_size;
	
	/**
	 * Align the memory for the 2MB.
	 */
	hpi->num_pages[0] =  left_mem / huge_page_size;
	internal_config.num_hugepage_sizes = 1;
	printf("%s: left memory: %llu huge pages of size: %d and count: %d\n",
		__func__, left_mem, huge_page_size, hpi->num_pages[0]);

	return 0;
}
