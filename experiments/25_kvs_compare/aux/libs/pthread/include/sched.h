/* SPDX-License-Identifier: LGPL-2.0-or-later */
/*
 *      Unikraft port of POSIX Threads Library for embedded systems
 *      Copyright(C) 2019, University Politehnica of Bucharest
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

#ifndef __PTHREAD_EMBEDDED__GLUE__SCHED_H__
#define __PTHREAD_EMBEDDED__GLUE__SCHED_H__

#include <stddef.h>
#include_next <sched.h>

#define CPU_SETSIZE 128
typedef struct cpu_set_t {
	unsigned long __bits[CPU_SETSIZE / sizeof(long)];
} cpu_set_t;

#ifdef CONFIG_LIBPTHREAD_CPUSET_OPS

void *memset (void *, int, size_t);
#define __CPU_op_S(i, size, set, op) ( (i)/8U >= (size) ? 0 : \
    (((unsigned long *)(set))[(i)/8/sizeof(long)] op (1UL<<((i)%(8*sizeof(long))))) )
#define CPU_SET_S(i, size, set) __CPU_op_S(i, size, set, |=)
#define CPU_CLR_S(i, size, set) __CPU_op_S(i, size, set, &=~)
#define CPU_ISSET_S(i, size, set) __CPU_op_S(i, size, set, &)
#define __CPU_op_func_S(func, op) \
static __inline void __CPU_##func##_S(size_t __size, cpu_set_t *__dest, \
    const cpu_set_t *__src1, const cpu_set_t *__src2) \
{ \
    size_t __i; \
    for (__i=0; __i<__size/sizeof(long); __i++) \
        ((unsigned long *)__dest)[__i] = ((unsigned long *)__src1)[__i] \
            op ((unsigned long *)__src2)[__i] ; \
}
__CPU_op_func_S(AND, &)
__CPU_op_func_S(OR, |)
__CPU_op_func_S(XOR, ^)
#define CPU_AND_S(a,b,c,d) __CPU_AND_S(a,b,c,d)
#define CPU_OR_S(a,b,c,d) __CPU_OR_S(a,b,c,d)
#define CPU_XOR_S(a,b,c,d) __CPU_XOR_S(a,b,c,d)
#define CPU_COUNT_S(size,set) __sched_cpucount(size,set)
#define CPU_ZERO_S(size,set) memset(set,0,size)
#define CPU_EQUAL_S(size,set1,set2) (!memcmp(set1,set2,size))
#define CPU_ALLOC_SIZE(n) (sizeof(long) * ( (n)/(8*sizeof(long)) \
    + ((n)%(8*sizeof(long)) + 8*sizeof(long)-1)/(8*sizeof(long)) ) )
#define CPU_ALLOC(n) ((cpu_set_t *)calloc(1,CPU_ALLOC_SIZE(n)))
#define CPU_FREE(set) free(set)
#define CPU_SETSIZE 128
#define CPU_SET(i, set) CPU_SET_S(i,sizeof(cpu_set_t),set)
#define CPU_CLR(i, set) CPU_CLR_S(i,sizeof(cpu_set_t),set)
#define CPU_ISSET(i, set) CPU_ISSET_S(i,sizeof(cpu_set_t),set)
#define CPU_AND(d,s1,s2) CPU_AND_S(sizeof(cpu_set_t),d,s1,s2)
#define CPU_OR(d,s1,s2) CPU_OR_S(sizeof(cpu_set_t),d,s1,s2)
#define CPU_XOR(d,s1,s2) CPU_XOR_S(sizeof(cpu_set_t),d,s1,s2)
#define CPU_COUNT(set) CPU_COUNT_S(sizeof(cpu_set_t),set)
#define CPU_ZERO(set) CPU_ZERO_S(sizeof(cpu_set_t),set)
#define CPU_EQUAL(s1,s2) CPU_EQUAL_S(sizeof(cpu_set_t),s1,s2)

#endif /* CONFIG_LIBPTHREAD_CPUSET_OPS */

#endif
