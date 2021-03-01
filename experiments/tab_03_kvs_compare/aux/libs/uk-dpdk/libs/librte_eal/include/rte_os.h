#ifndef RTE_OS_H
#define RTE_OS_H

#include <uk/init.h>

#define LOG 1
#define BUS 2
#define CLASS 3
#define LAST   9

#define RTE_INIT_PRIO(fn, prio)	\
	static void fn(void);		\
	static int init_ ## fn(void) {	\
		printf("init function %s\n", __func__);\
		fn();				\
		return 0;			\
	}					\
	uk_late_initcall_prio(init_ ## fn, prio);	\
	static void fn(void)

#endif /* RTE_OS_H */
