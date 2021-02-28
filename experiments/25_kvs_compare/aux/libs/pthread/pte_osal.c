/* SPDX-License-Identifier: LGPL-2.0-or-later */
/*
 *      Unikraft port of POSIX Threads Library for embedded systems
 *      Copyright(C) 2019 Costin Lupu, University Politehnica of Bucharest
 *
 *      This library is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU Lesser General Public
 *      License as published by the Free Software Foundation; either
 *      version 2 of the License, or (at your option) any later version.
 *
 *      This library is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *      Lesser General Public License for more details.
 *
 *      You should have received a copy of the GNU Lesser General Public
 *      License along with this library in the file COPYING.LIB;
 *      if not, write to the Free Software Foundation, Inc.,
 *      59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

/*
 * This port is derived from hermit/pte_osal.c.
 */

#include <string.h>
#include <uk/essentials.h>
#include <uk/arch/time.h>
#include <uk/arch/atomic.h>
#include <uk/print.h>
#include <uk/thread.h>
#include "pte_osal.h"
#include "pthread.h"
#include "tls-helper.h"


typedef struct {
	/* thread routine */
	pte_osThreadEntryPoint entry_point;
	/* thread routine arguments */
	void *argv;
	/* Unikraft thread */
	struct uk_thread *uk_thread;
	/* TLS */
	void *tls;
	/* Semaphore for triggering thread start */
	struct uk_semaphore start_sem;
	/* Semaphore for cancellation */
	struct uk_semaphore cancel_sem;
	/* Is non-zero if thread exited */
	int done;
} pte_thread_data_t;


/****************************************************************************
 *
 * Initialization
 *
 ***************************************************************************/

int __constructor pthread_constructor(void)
{
	uk_pr_debug("pthread-embedded constructor\n");
	return pthread_init();
}

pte_osResult pte_osInit(void)
{
	pte_osResult result = PTE_OS_OK;
	pte_thread_data_t *ptd;
	struct uk_thread *crnt;

	/* Allocate and initialize TLS support */
	result = pteTlsGlobalInit(CONFIG_LIBPTHREAD_EMBEDDED_MAX_TLS);
	if (result != PTE_OS_OK) {
		uk_pr_err("Could not init global TLS");
		goto out;
	}

	/* Create a ptd for initializing thread. */
	ptd = calloc(1, sizeof(pte_thread_data_t));
	if (ptd == NULL) {
		result = PTE_OS_NO_RESOURCES;
		goto out;
	}

	ptd->tls = pteTlsThreadInit();
	if (ptd->tls == NULL) {
		uk_pr_err("Could not init TLS");
		free(ptd);
		result = PTE_OS_NO_RESOURCES;
		goto out;
	}

	crnt = uk_thread_current();
	crnt->prv = ptd;
	ptd->uk_thread = crnt;

out:
	return result;
}

/****************************************************************************
 *
 * Threads
 *
 ***************************************************************************/

static pte_thread_data_t *handle_to_ptd(pte_osThreadHandle h)
{
	return h->prv;
}

static pte_thread_data_t *current_ptd(void)
{
	return uk_thread_current()->prv;
}

static void uk_stub_thread_entry(void *argv)
{
	pte_thread_data_t *ptd = (pte_thread_data_t *) argv;

	/* wait for the resume command */
	uk_semaphore_down(&ptd->start_sem);

	ptd->entry_point(ptd->argv);
}

pte_osResult pte_osThreadCreate(pte_osThreadEntryPoint entry_point,
	int stack_size, int initial_prio, void *argv,
	pte_osThreadHandle *ph)
{
	pte_thread_data_t *ptd;

	ptd = malloc(sizeof(pte_thread_data_t));
	if (!ptd)
		return PTE_OS_NO_RESOURCES;

	ptd->entry_point = entry_point;
	ptd->argv = argv;

	/* Allocate TLS structure for this thread. */
	ptd->tls = pteTlsThreadInit();
	if (ptd->tls == NULL) {
		uk_pr_err("Could not allocate TLS\n");
		free(ptd);
		return PTE_OS_NO_RESOURCES;
	}

	uk_semaphore_init(&ptd->start_sem, 0);
	uk_semaphore_init(&ptd->cancel_sem, 0);
	ptd->done = 0;

	ptd->uk_thread = uk_thread_create_attr(NULL, NULL,
		uk_stub_thread_entry, ptd);
	if (ptd->uk_thread == NULL) {
		pteTlsThreadDestroy(ptd->tls);
		free(ptd);
		return PTE_OS_NO_RESOURCES;
	}

	ptd->uk_thread->prv = ptd;

	*ph = ptd->uk_thread;

	return PTE_OS_OK;
}

pte_osResult pte_osThreadStart(pte_osThreadHandle h)
{
	pte_thread_data_t *ptd = handle_to_ptd(h);

	/* wake up thread */
	uk_semaphore_up(&ptd->start_sem);

	return 0;
}

pte_osResult pte_osThreadDelete(pte_osThreadHandle h)
{
	pte_thread_data_t *ptd = handle_to_ptd(h);

	/* free resources */
	pteTlsThreadDestroy(ptd->tls);
	free(ptd);

	return PTE_OS_OK;
}

pte_osResult pte_osThreadExitAndDelete(pte_osThreadHandle h)
{
	if (h->sched)
		uk_thread_kill(h);
	pte_osThreadDelete(h);

	return PTE_OS_OK;
}

void pte_osThreadExit(void)
{
	pte_thread_data_t *ptd = current_ptd();

	ptd->done = 1;
	uk_sched_thread_exit();
}

pte_osResult pte_osThreadWaitForEnd(pte_osThreadHandle h)
{
	pte_thread_data_t *ptd = handle_to_ptd(h);
	pte_thread_data_t *self_ptd = current_ptd();

	while (1) {
		if (ptd->done) {
			uk_thread_wait(ptd->uk_thread);
			return PTE_OS_OK;
		}

		if (self_ptd && self_ptd->cancel_sem.count > 0)
			return PTE_OS_INTERRUPTED;

		else
			uk_sched_yield();
	}
}

pte_osResult pte_osThreadCancel(pte_osThreadHandle h)
{
	pte_thread_data_t *ptd = handle_to_ptd(h);

	uk_semaphore_up(&ptd->cancel_sem);

	return PTE_OS_OK;
}

pte_osResult pte_osThreadCheckCancel(pte_osThreadHandle h)
{
	pte_thread_data_t *ptd = handle_to_ptd(h);

	if (ptd && ptd->cancel_sem.count > 0)
		return PTE_OS_INTERRUPTED;

	return PTE_OS_OK;
}

pte_osThreadHandle pte_osThreadGetHandle(void)
{
	return uk_thread_current();
}

int pte_osThreadGetPriority(pte_osThreadHandle h)
{
	pte_thread_data_t *ptd = handle_to_ptd(h);
	prio_t prio;

	int ret = uk_thread_get_prio(ptd->uk_thread, &prio);

	return ret ? PTE_OS_GENERAL_FAILURE : PTE_OS_OK;
}

pte_osResult pte_osThreadSetPriority(pte_osThreadHandle h, int new_prio)
{
	pte_thread_data_t *ptd = handle_to_ptd(h);

	int ret = uk_thread_set_prio(ptd->uk_thread, new_prio);

	return ret ? PTE_OS_GENERAL_FAILURE : PTE_OS_OK;
}

void pte_osThreadSleep(unsigned int msecs)
{
	__nsec nsec = ukarch_time_msec_to_nsec(msecs);

	uk_sched_thread_sleep(nsec);
}

int pte_osThreadGetMinPriority(void)
{
	return UK_THREAD_ATTR_PRIO_MIN;
}

int pte_osThreadGetMaxPriority(void)
{
	return UK_THREAD_ATTR_PRIO_MAX;
}

int pte_osThreadGetDefaultPriority(void)
{
	return UK_THREAD_ATTR_PRIO_DEFAULT;
}

/****************************************************************************
 *
 * Mutexes
 *
 ****************************************************************************/

pte_osResult pte_osMutexCreate(pte_osMutexHandle *ph)
{
	struct uk_mutex *m;

	if (!ph)
		return PTE_OS_INVALID_PARAM;

	m = malloc(sizeof(struct uk_mutex));
	if (!m)
		return PTE_OS_NO_RESOURCES;

	uk_mutex_init(m);

	*ph = m;

	return PTE_OS_OK;
}

pte_osResult pte_osMutexDelete(pte_osMutexHandle h)
{
	if (!h)
		return PTE_OS_INVALID_PARAM;

	free(h);

	return PTE_OS_OK;
}

pte_osResult pte_osMutexLock(pte_osMutexHandle h)
{
	if (!h)
		return PTE_OS_INVALID_PARAM;

	uk_mutex_lock(h);

	return PTE_OS_OK;
}

pte_osResult pte_osMutexTimedLock(pte_osMutexHandle h,
	unsigned int timeoutMsecs)
{
	return PTE_OS_GENERAL_FAILURE;
}


pte_osResult pte_osMutexUnlock(pte_osMutexHandle h)
{
	if (!h)
		return PTE_OS_INVALID_PARAM;

	uk_mutex_unlock(h);

	return PTE_OS_OK;
}

/****************************************************************************
 *
 * Semaphores
 *
 ***************************************************************************/

pte_osResult pte_osSemaphoreCreate(int init_value, pte_osSemaphoreHandle *ph)
{
	struct uk_semaphore *s;

	if (!ph)
		return PTE_OS_INVALID_PARAM;

	s = malloc(sizeof(struct uk_semaphore));
	if (!s)
		return PTE_OS_NO_RESOURCES;

	uk_semaphore_init(s, init_value);

	*ph = s;

	return PTE_OS_OK;
}

pte_osResult pte_osSemaphoreDelete(pte_osSemaphoreHandle h)
{
	if (!h)
		return PTE_OS_INVALID_PARAM;

	free(h);

	return PTE_OS_OK;
}

pte_osResult pte_osSemaphorePost(pte_osSemaphoreHandle h, int count)
{
	int i;

	if (!h)
		return PTE_OS_INVALID_PARAM;

	for (i = 0; i < count; i++)
		uk_semaphore_up(h);

	return PTE_OS_OK;
}

pte_osResult pte_osSemaphorePend(pte_osSemaphoreHandle h,
	unsigned int *ptimeout_msecs)
{
	__nsec timeout;

	if (!h)
		return PTE_OS_INVALID_PARAM;

	if (ptimeout_msecs) {
		timeout = ukarch_time_msec_to_nsec(*ptimeout_msecs);

		if (uk_semaphore_down_to(h, timeout) == __NSEC_MAX)
			return PTE_OS_TIMEOUT;

	} else
		uk_semaphore_down(h);

	return PTE_OS_OK;
}

pte_osResult pte_osSemaphoreCancellablePend(pte_osSemaphoreHandle h,
	unsigned int *ptimeout_msecs)
{
	pte_thread_data_t *ptd = current_ptd();
	pte_osResult result = PTE_OS_OK;
	__nsec timeout = 0, start_time = ukplat_monotonic_clock();

	if (ptimeout_msecs)
		timeout = ukarch_time_msec_to_nsec(*ptimeout_msecs);

	while (1) {
		if (uk_semaphore_down_try(h))
			/* semaphore is up */
			break;

		else if (timeout &&
			(ukplat_monotonic_clock() - start_time > timeout)) {
			/* The timeout expired */
			result = PTE_OS_TIMEOUT;
			break;

		} else if (ptd && ptd->cancel_sem.count > 0) {
			/* The thread was cancelled */
			result = PTE_OS_INTERRUPTED;
			break;

		} else
			/* Maybe next time... */
			uk_sched_yield();
	}

	return result;
}

#if 0
/* We use macros instead */
/****************************************************************************
 *
 * Atomic Operations
 *
 ***************************************************************************/

static int atomic_add(int *ptarg, int val)
{
	return __atomic_add_fetch(ptarg, val, __ATOMIC_SEQ_CST);
}

int pte_osAtomicExchange(int *ptarg, int val)
{
	return ukarch_exchange_n(ptarg, val);
}

int pte_osAtomicCompareExchange(int *pdest, int exchange, int comp)
{
	int orig = *pdest;

	ukarch_compare_exchange_sync(pdest, comp, exchange);

	return orig;
}

int pte_osAtomicExchangeAdd(int volatile *paddend, int value)
{
	return ukarch_fetch_add(paddend, value);
}

int pte_osAtomicDecrement(int *pdest)
{
	return atomic_add(pdest, -1);
}

int pte_osAtomicIncrement(int *pdest)
{
	return atomic_add(pdest, 1);
}
#endif

/****************************************************************************
 *
 * Thread Local Storage
 *
 ***************************************************************************/

static void *current_tls(void)
{
	pte_thread_data_t *ptd = current_ptd();

	return ptd ? ptd->tls : NULL;
}

pte_osResult pte_osTlsSetValue(unsigned int key, void *value)
{
	return pteTlsSetValue(current_tls(), key, value);
}

void *pte_osTlsGetValue(unsigned int index)
{
	return (void *) pteTlsGetValue(current_tls(), index);
}

pte_osResult pte_osTlsAlloc(unsigned int *pkey)
{
	return pteTlsAlloc(pkey);
}

pte_osResult pte_osTlsFree(unsigned int index)
{
	return pteTlsFree(index);
}

/***************************************************************************
 *
 * Miscellaneous
 *
 ***************************************************************************/

int ftime(struct timeb *tb)
{
	__nsec now = ukplat_monotonic_clock();

	if (tb) {
		tb->time = ukarch_time_nsec_to_sec(now);
		tb->millitm = ukarch_time_nsec_to_msec(ukarch_time_subsec(now));
	}

	return 0;
}
