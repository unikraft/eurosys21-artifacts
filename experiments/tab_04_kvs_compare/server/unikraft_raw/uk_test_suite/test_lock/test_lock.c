#include <testsuite.h>
#include <test_assert.h>
#include <uk/mutex.h>
#include <uk/sched.h>
#include <uk/thread.h>
#include <stdio.h>

static void test_basic_rlock()
{
	struct uk_mutex mutex = {0};

	uk_mutex_lock(&mutex);
	uk_mutex_lock(&mutex);
	uk_mutex_lock(&mutex);
	uk_mutex_trylock(&mutex);
	
	printf("hello world from\n");

	uk_mutex_unlock(&mutex);
	uk_mutex_unlock(&mutex);
	uk_mutex_unlock(&mutex);
	uk_mutex_unlock(&mutex);
	
}


#define MAX_ITERATION 100
struct rlock {
	struct uk_mutex mutex;
	int id;
};

static void print_number(struct uk_thread *t, struct rlock *lock)
{
	int i = 0;

	int j = 0;
	while (lock->id < MAX_ITERATION) {
		for (j = 0; j < i; j++)
			uk_mutex_lock(&lock->mutex);	
		printf("%p -- %d\n", t, lock->id);
		lock->id++;
		for (j = 0; j < i; j++)
			uk_mutex_unlock(&lock->mutex);	

		uk_sched_yield();
	}
}

static void test_fn(void *arg)
{
	struct rlock *lock = (struct rlock *)arg;
	struct uk_thread *t1 = uk_thread_current();

	printf("thread %s -- %p\n", __func__, t1);
	print_number(uk_thread_current(), lock);
}

static void test_multithread_lock()
{
	struct uk_thread *t1;
	struct rlock lock = {0};

	printf("thread %s -- %p\n", __func__, uk_thread_current());

	t1 = uk_thread_create("test1", test_fn, &lock);
	print_number(uk_thread_current(), &lock);
}

static int test_uk_lock(void)
{
	test_basic_rlock();
	test_multithread_lock();
	return 0;
}
TESTSUITE_REGISTER(uklock_test, test_uk_lock);
