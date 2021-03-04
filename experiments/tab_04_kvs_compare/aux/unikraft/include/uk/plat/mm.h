/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Authors: Stefan Teodorescu <stefanl.teodorescu@gmail.com
 *
 * Copyright (c) 2020, University Politehnica of Bucharest. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * THIS HEADER MAY NOT BE EXTRACTED OR MODIFIED IN ANY WAY.
 */

#ifndef __UKPLAT_PT__
#define __UKPLAT_PT__

/* TODO: maybe change design here? (these flags are needed in uk/asm/mm.h) */
#define PAGE_PROT_READ  0x1
#define PAGE_PROT_WRITE 0x2
#define PAGE_PROT_EXEC  0x4

#include <uk/asm/mm.h>

#define PAGE_SIZE 0x1000
#define PAGE_SHIFT 12

/* TODO: these might work even for arm64 (have to check) */
static inline void write_l1_entry(unsigned long l1_table, size_t offset, unsigned long value)
{
    UK_ASSERT(PAGE_ALIGNED(l1_table));
    UK_ASSERT(offset < L1_PAGETABLE_ENTRIES);

#if defined(__XEN__) || defined(__XEN_TOOLS__)
    /* TODO */
#else
    *((unsigned long *) l1_table + offset) = value;
#endif
}

static inline void write_l2_entry(unsigned long l2_table, size_t offset, unsigned long value)
{
    UK_ASSERT(PAGE_ALIGNED(l2_table));
    UK_ASSERT(offset < L2_PAGETABLE_ENTRIES);

#if defined(__XEN__) || defined(__XEN_TOOLS__)
    /* TODO */
#else
    *((unsigned long *) l2_table + offset) = value;
#endif
}

static inline void write_l3_entry(unsigned long l3_table, size_t offset, unsigned long value)
{
    UK_ASSERT(PAGE_ALIGNED(l3_table));
    UK_ASSERT(offset < L3_PAGETABLE_ENTRIES);

#if defined(__XEN__) || defined(__XEN_TOOLS__)
    /* TODO */
#else
    *((unsigned long *) l3_table + offset) = value;
#endif
}

static inline void write_l4_entry(unsigned long l4_table, size_t offset, unsigned long value)
{
    UK_ASSERT(PAGE_ALIGNED(l4_table));
    UK_ASSERT(offset < L4_PAGETABLE_ENTRIES);

#if defined(__XEN__) || defined(__XEN_TOOLS__)
    /* TODO */
#else
    *((unsigned long *) l4_table + offset) = value;
#endif
}

static inline unsigned long get_l1_entry(unsigned long l1_table, size_t offset)
{
    UK_ASSERT(PAGE_ALIGNED(l1_table));
    UK_ASSERT(offset < L1_PAGETABLE_ENTRIES);

#if defined(__XEN__) || defined(__XEN_TOOLS__)
    /* TODO */
#else
    return *((unsigned long *) l1_table + offset);
#endif
}

static inline unsigned long get_l2_entry(unsigned long l2_table, size_t offset)
{
    UK_ASSERT(PAGE_ALIGNED(l2_table));
    UK_ASSERT(offset < L2_PAGETABLE_ENTRIES);

#if defined(__XEN__) || defined(__XEN_TOOLS__)
    /* TODO */
#else
    return *((unsigned long *) l2_table + offset);
#endif
}

static inline unsigned long get_l3_entry(unsigned long l3_table, size_t offset)
{
    UK_ASSERT(PAGE_ALIGNED(l3_table));
    UK_ASSERT(offset < L3_PAGETABLE_ENTRIES);

#if defined(__XEN__) || defined(__XEN_TOOLS__)
    /* TODO */
#else
    return *((unsigned long *) l3_table + offset);
#endif
}

static inline unsigned long get_l4_entry(unsigned long l4_table, size_t offset)
{
    UK_ASSERT(PAGE_ALIGNED(l4_table));
    UK_ASSERT(offset < L4_PAGETABLE_ENTRIES);

#if defined(__XEN__) || defined(__XEN_TOOLS__)
    /* TODO */
#else
    return *((unsigned long *) l4_table + offset);
#endif
}

int uk_page_map(unsigned long vaddr, unsigned long paddr, unsigned long prot);

int uk_page_unmap(unsigned long vaddr);

int uk_page_change_prot(unsigned long vaddr, unsigned long new_prot);

int can_map(unsigned long vaddr);

int uk_pt_init(unsigned long paddr_start, size_t len);

#endif /* __UKPLAT_PT__ */

