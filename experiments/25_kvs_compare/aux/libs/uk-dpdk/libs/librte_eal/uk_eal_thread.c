#include <uk/essentials.h>
#include <errno.h>
#include <sched.h>
#include <rte_per_lcore.h>
#include <rte_lcore.h>
#include <rte_memory.h>
#include <uk/assert.h>

RTE_DEFINE_PER_LCORE(unsigned, _lcore_id) = LCORE_ID_ANY;
RTE_DEFINE_PER_LCORE(unsigned, _socket_id) = (unsigned)SOCKET_ID_ANY;
RTE_DEFINE_PER_LCORE(rte_cpuset_t, _cpuset);

int rte_thread_setname(pthread_t id __unused, const char *name __unused)
{
	return -ENOSYS;
}

int rte_eal_remote_launch(int (*f)(void *), void *arg, unsigned slave_id)
{
	return -EINVAL;
}

static int eal_thread_set_affinity(void)
{
	unsigned lcore_id = rte_lcore_id();

	/* acquire system unique id  */
	rte_gettid();
        /* update EAL thread core affinity */
	return rte_thread_set_affinity(&lcore_config[lcore_id].cpuset);
}

void eal_thread_init_master(unsigned lcore_id)
{
	/* set the lcore ID in per-lcore memory area */
	RTE_PER_LCORE(_lcore_id) = lcore_id;

	/* set CPU affinity */
	if (eal_thread_set_affinity() < 0)
		UK_CRASH("Failed to set thread affinity\n");
}

int rte_sys_gettid(void)
{
	return 0;
}
