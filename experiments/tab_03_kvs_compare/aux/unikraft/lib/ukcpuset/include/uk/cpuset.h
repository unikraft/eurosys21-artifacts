#ifndef UK_CPUSET_H
#define UK_CPUSET_H

#include <stdint.h>
#include <string.h>
#include <uk/list.h>
#include <uk/essentials.h>
#include <uk/bitmap.h>


typedef uint32_t cpuwhich_t;
typedef int cpusetid_t;

#define CPU_MAXSIZE 256
BITSET_DEFINE(_cpuset, CPU_MAXSIZE);

typedef struct _cpuset  cpuset_t;

/**
 * List head of the setlist.
 */
static UK_LIST_HEAD(setlist);

#define UK_CPU_CLR(n, p)           uk_clear_bit(n, (void *)p)
#define UK_CPU_ISSET(n, p)         uk_test_bit(n, (void *)p)
#define UK_CPU_SET(n, p)           uk_set_bit(n, (void *)p)
#define UK_CPU_ZERO(p)             uk_bitmap_zero((void *)p, CPU_MAXSIZE)
#define UK_CPU_FILL(p)             uk_bitmap_fill(p, CPU_MAXSIZE)
#define UK_CPU_SETOF(n, p)         uk_bitmap_set((void *)p, n, CPU_MAXSIZE)
#define UK_CPU_EMPTY(p)            uk_bitmap_empty((void *)p, CPU_MAXSIZE)
#define UK_CPU_ISFULLSET(p)        uk_bitmap_full(p, CPU_MAXSIZE)
#define UK_CPU_SUBSET(p, c)        uk_bitmap_equal(p, c, CPU_MAXSIZE)
#define UK_CPU_OVERLAP(p, c)       uk_bitmap_equal(p, c, CPU_MAXSIZE)
#define UK_CPU_CMP(p, c)           uk_bitmap_equal(p, c, CPU_MAXSIZE)
#define UK_CPU_OR(d, s)            uk_bitmap_or(d, d, s, CPU_MAXSIZE)
#define UK_CPU_AND(d, s)           uk_bitmap_and(d, d, s, CPU_MAXSIZE)
#define UK_CPU_NAND(d, s)          uk_bitmap_and(d, d,\
				uk_bitmap_complement(s, s, CPU_MAXSIZE), \
				CPU_MAXSIZE)
#define UK_CPU_COUNT(p)            uk_bitmap_weight(p, CPU_MAXSIZE)
//#define CPU_CLR_ATOMIC(n, p)        BIT_CLR_ATOMIC(CPU_SETSIZE, n, p)
//#define CPU_SET_ATOMIC(n, p)        BIT_SET_ATOMIC(CPU_SETSIZE, n, p)
//#define CPU_SET_ATOMIC_ACQ(n, p)    BIT_SET_ATOMIC_ACQ(CPU_SETSIZE, n, p)
//#define CPU_AND_ATOMIC(n, p)        BIT_AND_ATOMIC(CPU_SETSIZE, n, p)
//#define CPU_OR_ATOMIC(d, s)     BIT_OR_ATOMIC(CPU_SETSIZE, d, s)
//#define CPU_COPY_STORE_REL(f, t)    BIT_COPY_STORE_REL(CPU_SETSIZE, f, t)
//#define CPU_FFS(p)          BIT_FFS(CPU_SETSIZE, p)
//#define CPUSET_FSET         BITSET_FSET(_NCPUWORDS)
//#define CPUSET_T_INITIALIZER        BITSET_T_INITIALIZER

/**
 * cpusets encapsulate cpu binding information for one or more threads.
 *
 *  a - Accessed with atomics.
 *  s - Set at creation, never modified.  Only a ref required to read.
 *  c - Locked internally by a cpuset lock.
 *
 * The bitmask is only modified while holding the cpuset lock.  It may be
 * read while only a reference is held but the consumer must be prepared
 * to deal with inconsistent results.
 */
struct cpuset {
	cpuset_t cs_mask;    /* bitmask of valid cpus. */
#ifdef CONFIG_NUMA
	struct domainset *cs_domain; /* (c) NUMA policy. */
#endif /* CONFIG_NUMA */
	volatile uint32_t cs_ref;     /* (a) Reference count. */
	int         cs_flags;   /* (s) Flags from below. */
	cpusetid_t  cs_id;      /* (s) Id or INVALID. */
	struct cpuset *cs_parent; /* (s) Pointer to our parent. */
	struct uk_list_head cs_link;    /* (c) All identified sets. */
	struct uk_list_head cs_siblings;    /* (c) Sibling set link. */
	struct uk_list_head cs_children;    /* (c) List of children. */
};

typedef cpuset_t cpu_set_t;

#ifdef CONFIG_CPUSET_IMPL
int cpuset(cpusetid_t *);
int cpuset_setid(cpuwhich_t, id_t, cpusetid_t);
int cpuset_getid(cpulevel_t, cpuwhich_t, id_t, cpusetid_t *);
int cpuset_getaffinity(cpulevel_t, cpuwhich_t, id_t, size_t, cpuset_t *);
int cpuset_setaffinity(cpulevel_t, cpuwhich_t, id_t, size_t, const cpuset_t *);
#endif /* CONFIG_CPUSET_IMPL */
#endif /* UK_CPUSET_H */
