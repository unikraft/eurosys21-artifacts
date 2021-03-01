#include <uk/essentials.h>
#include <rte_config.h>
#include <rte_lcore.h>
#include <eal_thread.h>
#include <uk/print.h>
/**
 * TODO:
 * In a unikernel with nosmp, we support for only a single core.
 * Our platform does not support for querying this information. The first
 * approach is to hard code configuration value.
 */
int eal_cpu_detected(int lcore_id)
{
	uk_pr_debug("Detecting core: %d\n", lcore_id);
	return (lcore_id == 0) ? 1 : 0;
}

unsigned eal_cpu_core_id(__unused unsigned lcore_id)
{
	return 0;
}


unsigned eal_cpu_socket_id(unsigned cpu_id)
{
	if (cpu_id == 0)
		return 0;
	else
		return -1;
}
