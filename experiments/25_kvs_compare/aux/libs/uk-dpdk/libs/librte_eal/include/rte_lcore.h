#ifndef UK_RTE_LCORE_H
#define UK_RTE_LCORE_H

#include <sched.h>

typedef cpu_set_t rte_cpuset_t;

#define RTE_CPU_AND(dst, src1, src2) CPU_AND(dst, src1, src2)
#define RTE_CPU_OR(dst, src1, src2) CPU_OR(dst, src1, src2)
#define RTE_CPU_FILL(set) do \
	{ \
		unsigned int i; \
		CPU_ZERO(set); \
		for (i = 0; i < CPU_SETSIZE; i++) \
			CPU_SET(i, set); \
	} while (0)
#define RTE_CPU_NOT(dst, src) do \
	{ \
		cpu_set_t tmp; \
		RTE_CPU_FILL(&tmp); \
		CPU_XOR(dst, &tmp, src); \
	} while (0)


#include_next <rte_lcore.h>

#endif /* UK_RTE_LCORE_H */
