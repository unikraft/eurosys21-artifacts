#ifndef UK_RTE_CONFIG_H
#define UK_RTE_CONFIG_H

#include <uk/config.h>
#include <_rte_config.h>

/**
 * Define Unikraft specific configuration
 */
/**
 * TODO:
 * Add these to Config.uk
 */
#define RTE_EAL_PMD_PATH ""
#define RTE_MAX_NUMA_NODES 1
#define RTE_MAX_LCORE 1
#define RTE_CACHE_LINE_SIZE 64 
#define RTE_MAX_ETHPORTS CONFIG_RTE_MAX_ETHPORTS

/**
 * TODO:
 * DPDK Fetches this information from defconfig. We need a similar scheme.
 */
#define RTE_VER_YEAR 19
#define RTE_VER_MONTH 8
#define RTE_VER_MINOR 0
#define RTE_VER_SUFFIX ""
#define RTE_VER_RELEASE 0

#define RTE_LOG_DP_LEVEL RTE_LOG_DEBUG
#define RTE_UK_MEMSIZE	(128ULL * 1024ULL * 1024ULL)

#endif /* UK_RTE_CONFIG_H */
