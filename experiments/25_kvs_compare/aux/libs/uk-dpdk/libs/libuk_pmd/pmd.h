#ifndef PMD_H
#define PMD_H

#include <uk/netdev.h>
#include <rte_device.h>

struct uk_pmd {
	struct uk_list_head next;
}

#endif /* PMD_H */
