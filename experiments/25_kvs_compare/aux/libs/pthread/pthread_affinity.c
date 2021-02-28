#define _GNU_SOURCE
#include <pthread.h>
#include <errno.h>
#include <uk/assert.h>
#include <sched.h>

/**
 * TODO:
 * Add a proper implementation for the pthread_getaffinity_np
 */
int pthread_getaffinity_np(pthread_t thread, size_t cpusetsize,
			   cpu_set_t *cpuset)
{
	UK_ASSERT(cpuset);

	CPU_SET(0, cpuset);
	return 0;
}

int pthread_setaffinity_np(pthread_t thread, size_t cpusetsize,
			   const cpu_set_t *cpuset)
{
	if (CPU_ISSET(0, cpuset))
		return 0;

	return -EINVAL;
}
