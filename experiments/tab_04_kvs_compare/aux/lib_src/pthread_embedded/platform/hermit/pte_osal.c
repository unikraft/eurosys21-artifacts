/*
 *      HermitCore port of POSIX Threads Library for embedded systems
 *      Copyright(C) 2015 Stefan Lankes, RWTH Aachen Univeristy
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
 * This port is derived from psp_osal.c.
 */

#include <stdio.h>
#include <stdlib.h>
#include <reent.h>
#include <string.h>
#include <stdatomic.h>
#include "pte_osal.h"
#include "pthread.h"
#include "syscall.h"

#include "tls-helper.h"

#define DEFAULT_STACK_SIZE_BYTES 4096
#define HERMIT_MAX_TLS		32
#define MAX_HERMIT_UID		32

#ifndef SIGTERM
#define SIGTERM 1
#endif

#if 1
int kprintf(const char*, ...);

#define HERMIT_DEBUG(...) kprintf(__VA_ARGS__);
#else
#define HERMIT_DEBUG(x)
#endif

#ifndef TIMER_FREQ
#define TIMER_FREQ	100
#endif

/* defined in crt0.o */
uint64_t get_ticks(void);

/* newlibs' helper function to initialize signal handling */
int _init_signal_r(struct _reent*);

/* TLS key used to access hermitThreadData struct for reach thread. */
static unsigned int threadDataKey;

/*
 * Data stored on a per-thread basis - allocated in pte_osThreadCreate
 * and freed in pte_osThreadDelete.
 */
typedef struct
  {
    /* Entry point and parameters to thread's main function */
    pte_osThreadEntryPoint entryPoint;
    void * argv;

    struct _reent* myreent;
    void* pTls;

    /* id of the thread */
    tid_t id;

    /* Semaphore to start and to stop the thread */
    pte_osSemaphoreHandle start_sem;
    pte_osSemaphoreHandle stop_sem;

    int done;
  } hermitThreadData;

static __thread void* globalHandle = NULL;
static __thread void* globalTls = NULL;
static __thread struct _reent* __myreent_ptr = NULL;

static inline tid_t gettid(void)
{
  return sys_getpid();
}

struct _reent * __getreent(void)
{
  return __myreent_ptr;
}

/* Declare helper function to terminate the current thread */
void NORETURN do_exit(int arg);

static void __reent_init(void)
{
  /*
   * prepare newlib to support reentrant calls
   */
  __myreent_ptr = _impure_ptr;

  /* initialize pthread library */
  pthread_init();
}

/*
 * create pointer for preinit_array to initialize the reentrant version of the libc
 */
__attribute__((section(".preinit_array"))) typeof(__reent_init) *__pte_preinit = __reent_init;

/* A new thread's stub entry point.  It retrieves the real entry point from the per thread control
 * data as well as any parameters to this function, and then calls the entry point.
 */
static void hermitStubThreadEntry(void *argv)
{
  //int ret;
  hermitThreadData *pThreadData = (hermitThreadData *) argv;

  if (!pThreadData || !pThreadData->myreent)
    do_exit(-PTE_OS_NO_RESOURCES);

  /* prepare newlib to support reentrant calls */
  __myreent_ptr = pThreadData->myreent;
  _REENT_INIT_PTR(pThreadData->myreent);

  /* initialize basic signal handling */
  _init_signal_r(pThreadData->myreent);

  pThreadData->id = gettid();
  globalHandle = (void*) pThreadData;

  /* Allocate TLS structure for this thread. */
  pThreadData->pTls = globalTls = pteTlsThreadInit();
  if (globalTls == NULL)
    {
      HERMIT_DEBUG("pteTlsThreadInit: PTE_OS_NO_RESOURCES\n");
      do_exit(-PTE_OS_NO_RESOURCES);
    }

  /* wait for the resume command */
  pte_osSemaphorePend(pThreadData->start_sem, NULL);

  /*ret =*/ (*(pThreadData->entryPoint))(pThreadData->argv);

  pte_osThreadExit();

  while(1);
}

/****************************************************************************
 *
 * Initialization
 *
 ***************************************************************************/

pte_osResult pte_osInit(void)
{
  pte_osResult result;
  hermitThreadData *pThreadData;

  /* Allocate and initialize TLS support */
  result = pteTlsGlobalInit(HERMIT_MAX_TLS);

  if (result == PTE_OS_OK)
  {

    /* Allocate a key that we use to store control information (e.g. cancellation semaphore) per thread */
    result = pteTlsAlloc(&threadDataKey);

    if (result == PTE_OS_OK)
      {

	/* Initialize the structure used to emulate TLS for
	 * non-POSIX threads
	 */
	globalTls = pteTlsThreadInit();

	/* Also create a "thread data" structure for a single non-POSIX thread. */

	/* Allocate some memory for our per-thread control data.  We use this for:
	 * 1. Entry point and parameters for the user thread's main function.
	 * 2. Semaphore used for thread cancellation.
	 */
	pThreadData = (hermitThreadData *) malloc(sizeof(hermitThreadData));
	globalHandle = pThreadData;

	if (pThreadData == NULL)
	  {
            pteTlsThreadDestroy(globalTls);
	    result = PTE_OS_NO_RESOURCES;
	  }
	else
	  {
            memset(pThreadData, 0, sizeof(hermitThreadData));
            pThreadData->id = gettid();
            pThreadData->myreent = _impure_ptr;
            pThreadData->pTls = globalTls;
	    result = PTE_OS_OK;
	  }
      }
  }

  return result;
}

/***************************************************************************
 *
 * Signal handling
 *
 **************************************************************************/

int pte_kill(pte_osThreadHandle threadId, int sig)
{
  hermitThreadData* pThreadData = (hermitThreadData*) threadId;

  return _kill_r(__getreent(), pThreadData->id, sig);
}

/****************************************************************************
 *
 * Threads
 *
 ***************************************************************************/

pte_osResult pte_osThreadCreate(pte_osThreadEntryPoint entryPoint,
                                int stackSize,
                                int initialPriority,
                                void *argv,
                                pte_osThreadHandle* ppte_osThreadHandle)
{
  hermitThreadData *pThreadData;

  /* Make sure that the stack we're going to allocate is big enough */
  if (stackSize < DEFAULT_STACK_SIZE_BYTES)
    {
      stackSize = DEFAULT_STACK_SIZE_BYTES;
    }

  pThreadData = (hermitThreadData*) malloc(sizeof(hermitThreadData));
  if (!pThreadData)
	return PTE_OS_NO_RESOURCES;


  pThreadData->entryPoint = entryPoint;
  pThreadData->argv = argv;
  pThreadData->start_sem = pThreadData->stop_sem = NULL;
  pThreadData->myreent = (struct _reent*) malloc(sizeof(struct _reent));
  pte_osSemaphoreCreate(0, &pThreadData->start_sem);
  pte_osSemaphoreCreate(0, &pThreadData->stop_sem);
  pThreadData->done = 0;
  //HERMIT_DEBUG("pThreadData %p\n", pThreadData);
  //HERMIT_DEBUG("init start_sem %p\n", pThreadData->start_sem);
  //HERMIT_DEBUG("init stop_sem %p\n", pThreadData->stop_sem);

  if (sys_clone(NULL, hermitStubThreadEntry, pThreadData)) {
    if (pThreadData->myreent)
      free(pThreadData->myreent);
    free(pThreadData);
    return PTE_OS_NO_RESOURCES;
  }

  *ppte_osThreadHandle = pThreadData;

  return PTE_OS_OK;
}


pte_osResult pte_osThreadStart(pte_osThreadHandle osThreadHandle)
{
  hermitThreadData *pThreadData = (hermitThreadData*) osThreadHandle;

  /* wake up thread */
  return pte_osSemaphorePost(pThreadData->start_sem, 1);
}


pte_osResult pte_osThreadDelete(pte_osThreadHandle handle)
{
  hermitThreadData *pThreadData = (hermitThreadData*) handle;

  /* free ressources */
  pte_osSemaphoreDelete(pThreadData->start_sem);
  pte_osSemaphoreDelete(pThreadData->stop_sem);
  pteTlsThreadDestroy(pThreadData->pTls);
  if (pThreadData->myreent && (pThreadData->myreent != _impure_ptr))
    free(pThreadData->myreent);
  free(pThreadData);

  return PTE_OS_OK;
}

pte_osResult pte_osThreadExitAndDelete(pte_osThreadHandle handle)
{
  hermitThreadData *pThreadData = (hermitThreadData*) handle;

  pte_kill(pThreadData, SIGTERM);
  pte_osThreadDelete(handle);

  return PTE_OS_OK;
}

void pte_osThreadExit(void)
{
  hermitThreadData *pThreadData = (hermitThreadData*) globalHandle;

  pThreadData->done = 1;
  pte_osSemaphorePost(pThreadData->stop_sem, 1);
  do_exit(0);
}

/*
 * This has to be cancellable, we currently ignore this behavious.
 */
pte_osResult pte_osThreadWaitForEnd(pte_osThreadHandle threadHandle)
{
  int ret;
  pte_osResult result = PTE_OS_OK;
  hermitThreadData *pThreadData = (hermitThreadData*) threadHandle;

  if (pThreadData->done)
    return PTE_OS_OK;

  /* wait for the resume command */
  ret = pte_osSemaphorePend(pThreadData->stop_sem, NULL);
  if (ret)
    result = PTE_OS_GENERAL_FAILURE;
 
  return result;
}

pte_osThreadHandle pte_osThreadGetHandle(void)
{
  return globalHandle;
}

int pte_osThreadGetPriority(pte_osThreadHandle threadHandle)
{
  int ret = sys_getprio(threadHandle);

  if (ret)
    return PTE_OS_GENERAL_FAILURE;

  return PTE_OS_OK;
}

pte_osResult pte_osThreadSetPriority(pte_osThreadHandle threadHandle, int newPriority)
{
  int ret = sys_setprio(threadHandle, newPriority);

  if (ret)
    return PTE_OS_GENERAL_FAILURE;

  return PTE_OS_OK;
}

void pte_osThreadSleep(unsigned int msecs)
{
  sys_msleep(msecs);
}

int pte_osThreadGetMinPriority(void)
{
	return OS_MIN_PRIO;
}

int pte_osThreadGetMaxPriority(void)
{
	return OS_MAX_PRIO;
}

int pte_osThreadGetDefaultPriority(void)
{
 	return OS_DEFAULT_PRIO;
}

/****************************************************************************
 *
 * Mutexes
 *
 ****************************************************************************/

pte_osResult pte_osMutexCreate(pte_osMutexHandle *pHandle)
{
  if (sys_sem_init((sem_t**) pHandle, 1/*, 1*/))
    return PTE_OS_NO_RESOURCES;	

  return PTE_OS_OK;
}

pte_osResult pte_osMutexDelete(pte_osMutexHandle handle)
{
  if (sys_sem_destroy(handle))
    return PTE_OS_NO_RESOURCES;

  return PTE_OS_OK;
}

pte_osResult pte_osMutexLock(pte_osMutexHandle handle)
{
  if (sys_sem_wait(handle))
    return PTE_OS_NO_RESOURCES;		

  return PTE_OS_OK;
}

pte_osResult pte_osMutexTimedLock(pte_osMutexHandle handle, unsigned int timeoutMsecs)
{
  if (sys_sem_timedwait(handle, timeoutMsecs))
    return PTE_OS_TIMEOUT;

  return PTE_OS_OK;
}


pte_osResult pte_osMutexUnlock(pte_osMutexHandle handle)
{
  if (sys_sem_post(handle))
    return PTE_OS_NO_RESOURCES;

  return PTE_OS_OK;
}

/****************************************************************************
 *
 * Semaphores
 *
 ***************************************************************************/

pte_osResult pte_osSemaphoreCreate(int initialValue, pte_osSemaphoreHandle *pHandle)
{
  if (sys_sem_init((sem_t**) pHandle, initialValue/*, SEM_VALUE_MAX*/))
    return PTE_OS_NO_RESOURCES;

  return PTE_OS_OK;
}

pte_osResult pte_osSemaphoreDelete(pte_osSemaphoreHandle handle)
{
  if (sys_sem_destroy(handle))
    return PTE_OS_NO_RESOURCES;

  return PTE_OS_OK;
}

pte_osResult pte_osSemaphorePost(pte_osSemaphoreHandle handle, int count)
{
  int i;

  for (i=0; i<count; i++) {
    if (sys_sem_post(handle))
       return PTE_OS_NO_RESOURCES;
  }

  return PTE_OS_OK;
}

pte_osResult pte_osSemaphorePend(pte_osSemaphoreHandle handle, unsigned int *pTimeoutMsecs)
{
  if (pTimeoutMsecs && *pTimeoutMsecs) {
    if (sys_sem_timedwait(handle, *pTimeoutMsecs))
      return PTE_OS_TIMEOUT;
  } else {
    if (sys_sem_wait(handle))
      return PTE_OS_NO_RESOURCES;
  }

  return PTE_OS_OK;
}


/*
 * Pend on a semaphore- and allow the pend to be cancelled.
 */
pte_osResult pte_osSemaphoreCancellablePend(pte_osSemaphoreHandle semHandle, unsigned int *pTimeout)
{
  unsigned int msec = 0;
  int ret;

  if (pTimeout)
    msec = *pTimeout;
  ret = sys_sem_cancelablewait(semHandle, msec);

  if (ret == -ETIME)
    return PTE_OS_TIMEOUT;
  if (ret)
    return PTE_OS_INTERRUPTED;

  return PTE_OS_OK;
}

/****************************************************************************
 *
 * Atomic Operations
 *
 ***************************************************************************/

 inline static int atomic_add(int *ptarg, int val)
 {
#ifdef __x86_64__
 	int res = val;
 	asm volatile("lock; xaddl %0, %1" : "=r"(val) : "m"(*ptarg), "0"(val) : "memory", "cc");
 	return res+val;
#else
	return __atomic_add_fetch(ptarg, val, __ATOMIC_SEQ_CST);
#endif
 }

int pte_osAtomicExchange(int *ptarg, int val)
{
#ifdef __x86_64__
 	asm volatile ("lock; xchgl %0, %1" : "=r"(val) : "m"(*ptarg), "0"(val) : "memory");

 	return val;
#else
	int ret;

	__atomic_exchange(ptarg, &val, &ret, __ATOMIC_SEQ_CST);

	return ret;
#endif
}

int pte_osAtomicCompareExchange(int *pdest, int exchange, int comp)
{
#ifdef __x86_64__
  int ret;

  asm volatile ("lock; cmpxchgl %2, %1" : "=a"(ret), "+m"(*pdest) : "r"(exchange), "0"(*pdest) : "memory", "cc");

  return ret;
#else
  __atomic_compare_exchange(pdest, &comp, &exchange, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
  return exchange;
#endif
}

int pte_osAtomicExchangeAdd(int volatile* pAddend, int value)
{
#ifdef __x86_64__
  asm volatile ("lock; xaddl %%eax, %2;" : "=a" (value) : "a" (value), "m" (*pAddend) : "memory", "cc");

  return value;
#else
  return __atomic_fetch_add(pAddend, value, __ATOMIC_SEQ_CST);
#endif
}

int pte_osAtomicDecrement(int *pdest)
{
  return atomic_add(pdest, -1);
}

int pte_osAtomicIncrement(int *pdest)
{
  return atomic_add(pdest, 1);
}

/****************************************************************************
 *
 * Thread Local Storage
 *
 ***************************************************************************/

pte_osResult pte_osTlsSetValue(unsigned int key, void * value)
{
  return pteTlsSetValue(globalTls, key, value);
}

void * pte_osTlsGetValue(unsigned int index)
{
  return (void *) pteTlsGetValue(globalTls, index);
}


pte_osResult pte_osTlsAlloc(unsigned int *pKey)
{
  return pteTlsAlloc(pKey);
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
  uint64_t ticks = sys_get_ticks();

  tb->time = ticks / TIMER_FREQ;
  tb->millitm = (ticks % TIMER_FREQ) * (TIMER_FREQ*1000);

  return 0;
}

/***************************************************************************
 *
 * Helper functions to protect newlib's heap and environment
 *
 ***************************************************************************/

void __sys_malloc_lock(void);
void __sys_malloc_unlock(void);
void __sys_env_lock(void);
void __sys_env_unlock(void);

void __malloc_lock(struct _reent *ptr)
{
  __sys_malloc_lock();
}

void __malloc_unlock(struct _reent *ptr)
{
  __sys_malloc_unlock();
}

void __env_lock(struct _reent *ptr)
{
  __sys_env_lock();
}

void __env_unlock(struct _reent *ptr)
{
  __sys_env_unlock();
}
