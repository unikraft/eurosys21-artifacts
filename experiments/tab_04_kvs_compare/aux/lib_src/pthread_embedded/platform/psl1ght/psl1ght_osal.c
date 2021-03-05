/*
 * psl1ght_osal.c
 *
 * Description:
 *
 * --------------------------------------------------------------------------
 *
 *      Pthreads-embedded (PTE) - POSIX Threads Library for embedded systems
 *      Copyright(C) 2008 Jason Schmidlapp
 *
 *      Contact Email: jschmidlapp@users.sourceforge.net
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pte_osal.h"
#include <lv2/thread.h>
#include "pthread.h"
#include "tls-helper.h"

/* For ftime */
#include <sys/time.h>
#include <sys/types.h>
#include <sys/timeb.h>

#include <sys/systime.h>
#include <sys/dbg.h>

#define MAX_PSL1GHT_UID 2048 // SWAG

#define DEFAULT_STACK_SIZE_BYTES 0x10000

#define PSL1GHT_MAX_TLS 32

#if 1
#define PSL1GHT_DEBUG(x) printf(x)
#else
#define PSL1GHT_DEBUG(x)
#endif

/* TLS key used to access psl1ghtThreadData struct for reach thread. */
static unsigned int threadDataKey;

/*
 * Data stored on a per-thread basis - allocated in pte_osThreadCreate
 * and freed in pte_osThreadDelete.
 */
typedef struct psl1ghtThreadData
  {
    /* Entry point and parameters to thread's main function */
    pte_osThreadEntryPoint entryPoint;
    void * argv;
    int priority;
    int ended;

    /* Semaphore used for cancellation.  Posted to by pte_osThreadCancel, 
       polled in pte_osSemaphoreCancellablePend */
    sys_sem_t cancelSem;

  } psl1ghtThreadData;


/* Structure used to emulate TLS on non-POSIX threads.  
 * This limits us to one non-POSIX thread that can
 * call pthread functions. */
static void *globalTls;

/* Helper functions */
static psl1ghtThreadData *getThreadData(sys_ppu_thread_t threadHandle);
static void *getTlsStructFromThread(sys_ppu_thread_t thid);

/* A new thread's stub entry point.  It retrieves the real entry point from the per thread control
 * data as well as any parameters to this function, and then calls the entry point.
 */
void psl1ghtStubThreadEntry (void *user_data)
{
  int result;
  psl1ghtThreadData *pThreadData = user_data;

  result = (*(pThreadData->entryPoint))(pThreadData->argv);

  pThreadData->ended = 1;
}

/****************************************************************************
 *
 * Initialization
 *
 ***************************************************************************/

pte_osResult pte_osInit(void)
{
  pte_osResult result;
  psl1ghtThreadData *pThreadData;

  /* Allocate and initialize TLS support */
  result = pteTlsGlobalInit(PSL1GHT_MAX_TLS);

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
	pThreadData = (psl1ghtThreadData *) malloc(sizeof(psl1ghtThreadData));
	
	if (pThreadData == NULL)
	  {
	    result = PTE_OS_NO_RESOURCES;
	  }
	else
	  {
            sys_sem_attr_t attr;
	    /* Save a pointer to our per-thread control data as a TLS value */
	    pteTlsSetValue(globalTls, threadDataKey, pThreadData);
	
	    /* Create a semaphore used to cancel threads */
	    attr.attr_protocol = SYS_SEM_ATTR_PROTOCOL;
            attr.attr_pshared = SYS_SEM_ATTR_PSHARED;
            attr.key = 0;
            attr.flags = 0;
	    snprintf(attr.name, sizeof(attr.name), "cnclGlob");
            if (sysSemCreate(&pThreadData->cancelSem, &attr, 0, 255) != 0)
              {
                free (pThreadData);
                result = PTE_OS_NO_RESOURCES;
              }

	    result = PTE_OS_OK;
	  }
      }
  }

  return result;
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
  char threadName[28];
  static int threadNum = 1;
  int psl1ghtAttr;
  void *pTls;
  sys_ppu_thread_t threadId;
  pte_osResult result;
  psl1ghtThreadData *pThreadData;
  sys_sem_attr_t sem_attr;
  s32 ret;

  if (threadNum++ > MAX_PSL1GHT_UID)
    {
      threadNum = 0;
    }

  /* Make sure that the stack we're going to allocate is big enough */
  if (stackSize < DEFAULT_STACK_SIZE_BYTES)
    {
      stackSize = DEFAULT_STACK_SIZE_BYTES;
    }

  /* Allocate TLS structure for this thread. */
  pTls = pteTlsThreadInit();
  if (pTls == NULL)
    {
      PSL1GHT_DEBUG("pteTlsThreadInit: PTE_OS_NO_RESOURCES\n");
      result = PTE_OS_NO_RESOURCES;
      goto FAIL0;
    }

  /* Allocate some memory for our per-thread control data.  We use this for:
   * 1. Entry point and parameters for the user thread's main function.
   * 2. Semaphore used for thread cancellation.
   */
  pThreadData = (psl1ghtThreadData *) malloc(sizeof(psl1ghtThreadData));

  if (pThreadData == NULL)
    {
      pteTlsThreadDestroy(pTls);

      PSL1GHT_DEBUG("malloc(psl1ghtThreadData): PTE_OS_NO_RESOURCES\n");
      result = PTE_OS_NO_RESOURCES;
      goto FAIL0;
    }

  /* Save a pointer to our per-thread control data as a TLS value */
  pteTlsSetValue(pTls, threadDataKey, pThreadData);

  pThreadData->entryPoint = entryPoint;
  pThreadData->argv = argv;

  /* Create a semaphore used to cancel threads */
  sem_attr.attr_protocol = SYS_SEM_ATTR_PROTOCOL;
  sem_attr.attr_pshared = SYS_SEM_ATTR_PSHARED;
  sem_attr.key = 0;
  sem_attr.flags = 0;
  snprintf(sem_attr.name, sizeof(sem_attr.name), "cncl%04d", threadNum);
  ret = sysSemCreate(&pThreadData->cancelSem, &sem_attr, 0, 255);

  if (ret == 0)
    {
      /* In order to emulate TLS functionality, we append the address of the TLS structure that we
       * allocated above to the thread's name.  To set or get TLS values for this thread, the user
       * needs to get the name of the thread from the OS and then parse the name to extract
       * a pointer to the TLS structure.
       */
      snprintf(threadName, sizeof(threadName), "pthread%04d__%x", threadNum, (unsigned int) ((u64)pTls));

      // FIXME: joinable or not ?
      psl1ghtAttr = 0;
      pThreadData->priority = initialPriority;
      pThreadData->ended = 0;

      //  printf("%s %p %d %d %d\n",threadName, psl1ghtStubThreadEntry, initialPriority, stackSize, psl1ghtAttr);
      ret = sysThreadCreate(&threadId, psl1ghtStubThreadEntry, pThreadData,
          OS_MAX_PRIO, stackSize, psl1ghtAttr, threadName);
    }

  if (ret == 0x80010004)
    {
      free(pThreadData);
      pteTlsThreadDestroy(pTls);

      PSL1GHT_DEBUG("sceKernelCreateThread: PTE_OS_NO_RESOURCES\n");
      result =  PTE_OS_NO_RESOURCES;
    }
  else if (ret != 0)
    {
      free(pThreadData);
      pteTlsThreadDestroy(pTls);

      PSL1GHT_DEBUG("sceKernelCreateThread: PTE_OS_GENERAL_FAILURE\n");
      result = PTE_OS_GENERAL_FAILURE;
    }
  else
    {
      *ppte_osThreadHandle = threadId;
      result = PTE_OS_OK;
    }

FAIL0:
  return result;
}


pte_osResult pte_osThreadStart(pte_osThreadHandle osThreadHandle)
{
  psl1ghtThreadData *pThreadData = getThreadData(osThreadHandle);

  sysThreadSetPriority(osThreadHandle, pThreadData->priority);

  return PTE_OS_OK;
}


pte_osResult pte_osThreadDelete(pte_osThreadHandle handle)
{
  psl1ghtThreadData *pThreadData;
  void *pTls;

  pTls = getTlsStructFromThread(handle);

  pThreadData = getThreadData(handle);

  sysSemDestroy(pThreadData->cancelSem);

  free(pThreadData);

  pteTlsThreadDestroy(pTls);

  return PTE_OS_OK;
}

void pte_osThreadExit()
{
  sysThreadExit (0);
}

pte_osResult pte_osThreadExitAndDelete(pte_osThreadHandle handle)
{
  pte_osThreadExit ();

  pte_osThreadDelete(handle);

  return PTE_OS_OK;
}


/*
 * This has to be cancellable, so we can't just call sceKernelWaitThreadEnd.
 * Instead, poll on this in a loop, like we do for a cancellable semaphore.
 */
pte_osResult pte_osThreadWaitForEnd(pte_osThreadHandle threadHandle)
{
  pte_osResult osResult;
  psl1ghtThreadData *pThreadData;
  u64 result;

  pThreadData = getThreadData(threadHandle);

  if (pThreadData == NULL)
    {
      sysThreadJoin (threadHandle, &result);
      osResult = PTE_OS_OK;
    }
  else
    {
      while (1)
        {
          if (pThreadData->ended == 1)
            {
              /* Thread has ended */
              osResult = PTE_OS_OK;
              break;
            }
          else
            {
              s32 count;

	      if (sysSemGetValue (pThreadData->cancelSem, &count) == 0)
		{
		  if (count > 0)
		    {
		      osResult = PTE_OS_INTERRUPTED;
		      break;
		    }
		  else
		    {
		      /* Nothing found and not timed out yet; let's yield so we're not
		       * in busy loop.
		       */
                      sysThreadYield ();
		    }
		}
	      else
		{
		  osResult = PTE_OS_GENERAL_FAILURE;
		  break;
		}
            }
        }
    }


  return osResult;
}

pte_osThreadHandle pte_osThreadGetHandle(void)
{
  pte_osThreadHandle id;
  sysThreadGetId (&id);
  return id;
}

int pte_osThreadGetPriority(pte_osThreadHandle threadHandle)
{
  s32 priority;
  sysThreadGetPriority (threadHandle, &priority);
  return priority;
}

pte_osResult pte_osThreadSetPriority(pte_osThreadHandle threadHandle, int newPriority)
{
  sysThreadSetPriority (threadHandle, newPriority);

  return PTE_OS_OK;
}

pte_osResult pte_osThreadCancel(pte_osThreadHandle threadHandle)
{
  pte_osResult osResult;
  s32 result;
  psl1ghtThreadData *pThreadData;

  pThreadData = getThreadData(threadHandle);

  result = sysSemPost(pThreadData->cancelSem, 1);

  if (result == 0)
    {
      osResult = PTE_OS_OK;
    }
  else
    {
      osResult = PTE_OS_GENERAL_FAILURE;
    }

  return osResult;
}


pte_osResult pte_osThreadCheckCancel(pte_osThreadHandle threadHandle)
{
  psl1ghtThreadData *pThreadData;
  pte_osResult result;

  pThreadData = getThreadData(threadHandle);

  if (pThreadData != NULL)
    {
      s32 count;

      if (sysSemGetValue (pThreadData->cancelSem, &count) == 0)
	{
	  if (count > 0)
	    {
	      result = PTE_OS_INTERRUPTED;
	    }
	  else
	    {
	      result = PTE_OS_OK;
	    }
	}
      else
	{
	  /* sysSemGetValue returned an error */
	  result = PTE_OS_GENERAL_FAILURE;
	}
    }
  else
    {
      /* For some reason, we couldn't get thread data */
      result = PTE_OS_GENERAL_FAILURE;
    }

  return result;
}

void pte_osThreadSleep(unsigned int msecs)
{
  sysUsleep (msecs*1000);
}

int pte_osThreadGetMinPriority()
{
  return OS_MIN_PRIO;
}

int pte_osThreadGetMaxPriority()
{
  return OS_MAX_PRIO;
}

int pte_osThreadGetDefaultPriority()
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
  static int mutexCtr = 0;
  sys_mutex_attr_t attr;

  if (mutexCtr++ > MAX_PSL1GHT_UID)
    {
      mutexCtr = 0;
    }

  attr.attr_protocol = SYS_MUTEX_PROTOCOL_FIFO;
  attr.attr_recursive = SYS_MUTEX_ATTR_NOT_RECURSIVE;
  attr.attr_pshared = SYS_MUTEX_ATTR_PSHARED;
  attr.attr_adaptive = SYS_MUTEX_ATTR_ADAPTIVE;
  attr.key = 0;
  attr.flags = 0;
  snprintf(attr.name,sizeof(attr.name),"mtx%d",mutexCtr);

  sysMutexCreate (pHandle, &attr);

  return PTE_OS_OK;
}

pte_osResult pte_osMutexDelete(pte_osMutexHandle handle)
{
  sysMutexDestroy(handle);

  return PTE_OS_OK;
}



pte_osResult pte_osMutexLock(pte_osMutexHandle handle)
{
  sysMutexLock(handle, 0);

  return PTE_OS_OK;
}

pte_osResult pte_osMutexTimedLock(pte_osMutexHandle handle, unsigned int timeoutMsecs)
{
  pte_osResult result;
  u64 timeoutUsecs = timeoutMsecs*1000;

  s32 status = sysMutexLock(handle, timeoutUsecs);

  if (status != 0)
    {
      // Assume that any error from sysMutexLock was due to a timeout
      result = PTE_OS_TIMEOUT;
    }
  else
    {
      result = PTE_OS_OK;
    }

  return result;
}


pte_osResult pte_osMutexUnlock(pte_osMutexHandle handle)
{
  sysMutexUnlock(handle);

  return PTE_OS_OK;
}

/****************************************************************************
 *
 * Semaphores
 *
 ***************************************************************************/

pte_osResult pte_osSemaphoreCreate(int initialValue, pte_osSemaphoreHandle *pHandle)
{
  static int semCtr = 0;
  sys_sem_attr_t attr;

  if (semCtr++ > MAX_PSL1GHT_UID)
    {
      semCtr = 0;
    }

  attr.attr_protocol = SYS_SEM_ATTR_PROTOCOL;
  attr.attr_pshared = SYS_SEM_ATTR_PSHARED;
  attr.key = 0;
  attr.flags = 0;
  snprintf(attr.name,sizeof(attr.name),"sem%d",semCtr);

  sysSemCreate(pHandle, &attr, initialValue, SEM_VALUE_MAX);

  return PTE_OS_OK;
}

pte_osResult pte_osSemaphoreDelete(pte_osSemaphoreHandle handle)
{
  sysSemDestroy(handle);

  return PTE_OS_OK;
}


pte_osResult pte_osSemaphorePost(pte_osSemaphoreHandle handle, int count)
{
  sysSemPost(handle, count);

  return PTE_OS_OK;
}

pte_osResult pte_osSemaphorePend(pte_osSemaphoreHandle handle, unsigned int *pTimeoutMsecs)
{
  u64 timeoutUsecs;
  s32 result;
  pte_osResult osResult;

  if (pTimeoutMsecs == NULL)
    timeoutUsecs = 0;
  else
    timeoutUsecs = *pTimeoutMsecs * 1000;

  result = sysSemWait(handle, timeoutUsecs);

  if (result == 0)
    {
      osResult = PTE_OS_OK;
    }
  else if (result == 0x8001000b)
    {
      osResult = PTE_OS_TIMEOUT;
    }
  else
    {
      osResult = PTE_OS_GENERAL_FAILURE;
    }

  return osResult;
}


/*
 * Pend on a semaphore- and allow the pend to be cancelled.
 *
 * PS3 OS provides no functionality to asynchronously interrupt a blocked call.  We simulte
 * this by polling on the main semaphore and the cancellation semaphore and sleeping in a loop.
 */
pte_osResult pte_osSemaphoreCancellablePend(pte_osSemaphoreHandle semHandle, unsigned int *pTimeout)
{
  psl1ghtThreadData *pThreadData;
  sys_ppu_thread_t threadId;

  sysThreadGetId (&threadId);
  pThreadData = getThreadData(threadId);

  clock_t start_time;
  s32 result =  0;
  u64 timeout;
  unsigned char timeoutEnabled;

  start_time = clock();

  // clock() is in microseconds, timeout as passed in was in milliseconds
  if (pTimeout == NULL)
    {
      timeout = 0;
      timeoutEnabled = 0;
    }
  else
    {
      timeout = *pTimeout * 1000;
      timeoutEnabled = 1;
    }

  while (1)
    {
      int status;

      /* Poll semaphore */
      status = sysSemTryWait(semHandle);

      if (status == 0)
        {
          /* User semaphore posted to */
          result = PTE_OS_OK;
          break;
        }
      else if ((timeoutEnabled) && ((clock() - start_time) > timeout))
        {
          /* Timeout expired */
          result = PTE_OS_TIMEOUT;
          break;
        }
      else
        {

          if (pThreadData != NULL)
            {
              s32 count;
	      s32 osResult;

              osResult = sysSemGetValue (pThreadData->cancelSem, &count);

	      if (osResult == 0)
		{
		  if (count > 0)
		    {
		      result = PTE_OS_INTERRUPTED;
		      break;
		    }
		  else
		    {
		      /* Nothing found and not timed out yet; let's yield so we're not
		       * in busy loop.
		       */
                      sysThreadYield ();
		    }
		}
	      else
		{
		  result = PTE_OS_GENERAL_FAILURE;
		  break;
		}
            }
        }
    }

  return result;
}


/****************************************************************************
 *
 * Atomic Operations
 *
 ***************************************************************************/

int pte_osAtomicExchange(int *ptarg, int val)
{
  /* TODO */
  int origVal;

  origVal = *ptarg;

  *ptarg = val;

  return origVal;

}

int pte_osAtomicCompareExchange(int *pdest, int exchange, int comp)
{
  /* TODO */
  int origVal;

  origVal = *pdest;

  if (*pdest == comp)
    {
      *pdest = exchange;
    }


  return origVal;
}


int pte_osAtomicExchangeAdd(int volatile* pAddend, int value)
{
  int origVal;
  /* TODO */

  origVal = *pAddend;

  *pAddend += value;


  return origVal;
}

int pte_osAtomicDecrement(int *pdest)
{
  int val;

  /* TODO */

  (*pdest)--;
  val = *pdest;


  return val;
}

int pte_osAtomicIncrement(int *pdest)
{
  int val;

  /* TODO */

  (*pdest)++;
  val = *pdest;


  return val;
}

/****************************************************************************
 *
 * Helper functions
 *
 ***************************************************************************/

static psl1ghtThreadData *getThreadData(sys_ppu_thread_t threadHandle)
{
  psl1ghtThreadData *pThreadData;
  void *pTls;

  pTls = getTlsStructFromThread(threadHandle);

  pThreadData = (psl1ghtThreadData *) pteTlsGetValue(pTls, threadDataKey);

  return pThreadData;
}

static void *getTlsStructFromThread(sys_ppu_thread_t thid)
{
  unsigned int ptr;
  unsigned int thrNum;
  void *pTls;
  int numMatches;
  char name[64];

  sysDbgGetPPUThreadName (thid, name);

  numMatches = sscanf(name,"pthread%04d__%x", &thrNum, &ptr);

  /* If we were called from a pthread, use the TLS allocated when the thread
   * was created.  Otherwise, we were called from a non-pthread, so use the
   * "global".  This is a pretty bad hack, but necessary due to lack of TLS on PS3.
   */
  if (numMatches == 2)
    {
      pTls = (void *) ((u64)ptr);
    }
  else
    {
      pTls = globalTls;
    }

  return pTls;
}

/****************************************************************************
 *
 * Thread Local Storage
 *
 ***************************************************************************/

pte_osResult pte_osTlsSetValue(unsigned int key, void * value)
{
  void *pTls;
  sys_ppu_thread_t threadId;

  sysThreadGetId (&threadId);
  pTls = getTlsStructFromThread(threadId);

  return pteTlsSetValue(pTls, key, value);
}

void * pte_osTlsGetValue(unsigned int index)
{
  void *pTls;
  sys_ppu_thread_t threadId;

  sysThreadGetId (&threadId);

  pTls = getTlsStructFromThread(threadId);

  return (void *) pteTlsGetValue(pTls, index);

}


pte_osResult pte_osTlsAlloc(unsigned int *pKey)
{
  void * pTls;
  sys_ppu_thread_t threadId;

  sysThreadGetId (&threadId);
  pTls = getTlsStructFromThread(threadId);

  return pteTlsAlloc(pKey);

}

pte_osResult pte_osTlsFree(unsigned int index)
{
  return pteTlsFree(index);
}

/****************************************************************************
 *
 * Miscellaneous
 *
 ***************************************************************************/

int ftime(struct timeb *tb)
{
  struct timeval tv;
  struct timezone tz;

  gettimeofday(&tv, &tz);

  tb->time = tv.tv_sec;
  tb->millitm = tv.tv_usec / 1000;
  tb->timezone = tz.tz_minuteswest;
  tb->dstflag = tz.tz_dsttime;

  return 0;
}
