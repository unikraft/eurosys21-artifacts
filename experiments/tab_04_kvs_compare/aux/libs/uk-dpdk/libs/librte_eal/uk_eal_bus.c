#include <uk/bus.h>
#include <rte_bus.h>

#include "eal_private.h"
/**
 * Bus iterator to find a particular bus.
 *
 * This function compares each registered bus to find one that matches
 * the data passed as parameter.
 *
 * If the comparison function returns zero this function will stop iterating
 * over any more buses. To continue a search the bus of a previous search can
 * be passed via the start parameter.
 *
 * @param start
 *	Starting point for the iteration.
 *
 * @param cmp
 *	Comparison function.
 *
 * @param data
 *	 Data to pass to comparison function.
 *
 * @return
 *	 A pointer to a rte_bus structure or NULL in case no bus matches
 */
struct rte_bus *rte_bus_find(const struct rte_bus *start, rte_bus_cmp_t cmp,
			     const void *data)
{
	return NULL;
}

/**
 * Find the registered bus for a particular device.
 */
struct rte_bus *rte_bus_find_by_device(const struct rte_device *dev)
{
	return NULL;
}

/**
 * Find the registered bus for a given name.
 */
struct rte_bus *rte_bus_find_by_name(const char *busname)
{
	return NULL;
}

enum rte_iova_mode rte_bus_get_iommu_class(void)
{
	return  RTE_IOVA_DC;
}

struct rte_bus *rte_bus_find_by_device_name(const char *str)
{
	return NULL;
}
