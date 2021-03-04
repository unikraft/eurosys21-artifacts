/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Authors: Stefan Teodorescu <stefanl.teodorescu@gmail.com>
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

#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <uk/assert.h>
#include <uk/bitmap.h>
#include <uk/print.h>
#include <uk/plat/mm.h>

static unsigned long bitmap_start_addr;
static unsigned long bitmap_length;
static unsigned long pt_start_addr;
static unsigned long pt_length;

static void dump_mem(unsigned long start, unsigned long len)
{
    for (unsigned long *c = start; c < start + len; c += 2)
        uk_pr_info("0x%016lx:\t0x%016lx 0x%016lx\n", c, *c, *(c + 1));
}

static unsigned long get_free_vpage()
{
    unsigned long offset = uk_bitmap_find_next_zero_area(
            bitmap_start_addr, bitmap_length, 0, 1, 0);

    uk_bitmap_set(bitmap_start_addr, offset, 1);

    if (offset * PAGE_SIZE > pt_length)
        uk_pr_err("Filled up all available space for page tables\n");
    return pt_start_addr + (offset << PAGE_SHIFT);
}

static inline unsigned long alloc_l1_table()
{
    // This is an L2 entry, so we set L2 flags
    return get_free_vpage() | L2_PROT;
}

static inline unsigned long alloc_l2_table()
{
    // This is an L3 entry, so we set L3 flags
     return get_free_vpage() | L3_PROT;
}

static inline unsigned long alloc_l3_table()
{
    // This is an L4 entry, so we set L4 flags
    return get_free_vpage() | L4_PROT;
}

static inline void release_l1_table_if_unused(unsigned long l1_table, unsigned long l2_table)
{
    if (!PAGE_ALIGNED(l1_table)) {
        uk_pr_info("Table's address must be aligned to page size\n");
        return -1;
    }

    for (size_t i = 0; i < L1_PAGETABLE_ENTRIES; i++)
        if (PAGE_PRESENT(get_l1_entry(l1_table, i)))
            return;

    write_l2_entry(l2_table, L2_OFFSET(l1_table), 0x0);

    unsigned long offset = (l1_table - pt_start_addr) >> PAGE_SHIFT;
    uk_bitmap_set(bitmap_start_addr, offset, 0);
}

static inline void release_l2_table_if_unused(unsigned long l2_table, unsigned long l3_table)
{
    if (!PAGE_ALIGNED(l2_table)) {
        uk_pr_info("Table's address must be aligned to page size\n");
        return -1;
    }

    for (size_t i = 0; i < L2_PAGETABLE_ENTRIES; i++)
        if (PAGE_PRESENT(get_l2_entry(l2_table, i)))
            return;

    write_l3_entry(l3_table, L3_OFFSET(l2_table), 0x0);
}

static inline void release_l3_table_if_unused(unsigned long l3_table, unsigned long l4_table)
{
    if (!PAGE_ALIGNED(l3_table)) {
        uk_pr_info("Table's address must be aligned to page size\n");
        return -1;
    }

    for (size_t i = 0; i < L3_PAGETABLE_ENTRIES; i++)
        if (PAGE_PRESENT(get_l3_entry(l3_table, i)))
            return;

    write_l4_entry(l4_table, L4_OFFSET(l3_table), 0x0);
}

int uk_page_map(unsigned long vaddr, unsigned long paddr, unsigned long prot)
{
    if (!PAGE_ALIGNED(vaddr)) {
        uk_pr_info("Address must be aligned to page size\n");
        return -1;
    }

    // TODO: change linear mapping
    // paddr = get_free_ppage();
    if (paddr == -1)
        paddr = vaddr;


    // XXX: On 64-bits architectures (x86_64 and arm64) the hierarchical page
    // tables have a 4 level layout. This implementation will need a revision
    // when introducing support for 32-bits architectures, since there are only
    // 3 levels of page tables.
    unsigned long l4_table = ukarch_read_pt_base();

    unsigned long l4_entry = get_l4_entry(PTE_REMOVE_FLAGS(l4_table), L4_OFFSET(vaddr));
    uk_pr_info("L4 entry for addr 0x%08lx is 0x%08lx\n", vaddr, l4_entry);
    if (!PAGE_PRESENT(l4_entry)) {
        uk_pr_info("No entry 0x%08lx in L4 table for address 0x%08lx\n", l4_entry, vaddr);

        l4_entry = alloc_l3_table();
        uk_pr_info("Creating new L3 table at 0x%08lx\n", l4_entry);
        write_l4_entry(l4_table, L4_OFFSET(vaddr), l4_entry);
    }

    unsigned long l3_table = PTE_REMOVE_FLAGS(l4_entry);
    unsigned long l3_entry = get_l3_entry(l3_table, L3_OFFSET(vaddr));
    uk_pr_info("L3 entry for addr 0x%08lx is 0x%08lx\n", vaddr, l3_entry);
    if (!PAGE_PRESENT(l3_entry)) {
        uk_pr_info("No entry 0x%08lx in L3 table for address 0x%08lx\n", l3_entry, vaddr);

        l3_entry = alloc_l2_table();
        uk_pr_info("Creating new L2 table at 0x%08lx\n", l3_entry);
        write_l3_entry(l3_table, L3_OFFSET(vaddr), l3_entry);
    }
    if (PAGE_HUGE(l3_entry)) {
        uk_pr_info("Entry for 0x%08lx is mapped in a huge page (1G) with L3 table Entry 0x%08lx\n", vaddr, l3_entry);
    }

    unsigned long l2_table = PTE_REMOVE_FLAGS(l3_entry);
    unsigned long l2_entry = get_l2_entry(l2_table, L2_OFFSET(vaddr));
    uk_pr_info("L2 entry for addr 0x%08lx is 0x%08lx\n", vaddr, l2_entry);
    if (!PAGE_PRESENT(l2_entry)) {
        uk_pr_info("No entry 0x%08lx in L2 table for address 0x%08lx\n", l2_entry, vaddr);

        l2_entry = alloc_l1_table();
        uk_pr_info("Creating new L1 table at 0x%08lx\n", l2_entry);
        write_l2_entry(l2_table, L2_OFFSET(vaddr), l2_entry);
    }
    if (PAGE_LARGE(l2_entry)) {
        uk_pr_info("Entry for 0x%08lx is mapped in a large page (2M) with L2 3ntry 0x%08lx\n", vaddr, l2_entry);
    }

    unsigned long l1_table = PTE_REMOVE_FLAGS(l2_entry);
    unsigned long l1_entry = get_l1_entry(l1_table, L1_OFFSET(vaddr));
    uk_pr_info("L1 entry for addr 0x%08lx is 0x%08lx\n", vaddr, l1_entry);
    if (PAGE_PRESENT(l1_entry)) {
        uk_pr_info("Entry 0x%08lx already exists in L1 table for address 0x%08lx\n", l1_entry, vaddr);
    } else {
        write_l1_entry(l1_table, L1_OFFSET(vaddr), create_l1_entry(paddr, prot));
        uk_pr_info("L1 entry for addr 0x%08lx is 0x%08lx\n", vaddr, create_l1_entry(paddr, prot));
    }

    return 0;
}

int uk_page_unmap(unsigned long vaddr)
{
    if (!PAGE_ALIGNED(vaddr)) {
        uk_pr_info("Address must be aligned to page size\n");
        return -1;
    }

    unsigned long l4_table = PTE_REMOVE_FLAGS(ukarch_read_pt_base());

    unsigned long l4_entry = get_l4_entry(l4_table, L4_OFFSET(vaddr));
    uk_pr_info("L4 entry for addr 0x%08lx is 0x%08lx\n", vaddr, l4_entry);
    if (!PAGE_PRESENT(l4_entry)) {
        uk_pr_info("No entry 0x%08lx in L4 table for address 0x%08lx\n", l4_entry, vaddr);

        return 0;
    }

    unsigned long l3_table = PTE_REMOVE_FLAGS(l4_entry);
    unsigned long l3_entry = get_l3_entry(l3_table, L3_OFFSET(vaddr));
    uk_pr_info("L3 entry for addr 0x%08lx is 0x%08lx\n", vaddr, l3_entry);
    if (!PAGE_PRESENT(l3_entry)) {
        uk_pr_info("No entry 0x%08lx in L3 table for address 0x%08lx\n", l3_entry, vaddr);

        return 0;
    }
    if (PAGE_HUGE(l3_entry)) {
        uk_pr_info("Entry for 0x%08lx is mapped in a huge page (1G) with L3 entry 0x%08lx\n", vaddr, l3_entry);
    }

    unsigned long l2_table = PTE_REMOVE_FLAGS(l3_entry);
    unsigned long l2_entry = get_l2_entry(l2_table, L2_OFFSET(vaddr));
    uk_pr_info("L2 entry for addr 0x%08lx is 0x%08lx\n", vaddr, l2_entry);
    if (!PAGE_PRESENT(l2_entry)) {
        uk_pr_info("No entry 0x%08lx in L2 table for address 0x%08lx\n", l2_entry, vaddr);

        return 0;
    }
    if (PAGE_LARGE(l2_entry)) {
        uk_pr_info("Entry for 0x%08lx is mapped in a large page (2M) with L2 entry 0x%08lx\n", vaddr, l2_entry);
    }

    unsigned long l1_table = PTE_REMOVE_FLAGS(l2_entry);
    write_l1_entry(l1_table, L1_OFFSET(vaddr), 0x0);
    ukarch_flush_tlb_entry(vaddr);

    // TODO: release empty PTs
    release_l1_table_if_unused(l1_table, l2_table);
    release_l2_table_if_unused(l2_table, l3_table);
    release_l3_table_if_unused(l3_table, l4_table);

    return 0;
}

int uk_page_change_prot(unsigned long vaddr, unsigned long new_prot)
{
    if (!PAGE_ALIGNED(vaddr)) {
        uk_pr_info("Address must be aligned to page size\n");
        return -1;
    }

    unsigned long l4_table = PTE_REMOVE_FLAGS(ukarch_read_pt_base());

    unsigned long l4_entry = get_l4_entry(l4_table, L4_OFFSET(vaddr));
    uk_pr_info("L4 entry for addr 0x%08lx is 0x%08lx\n", vaddr, l4_entry);
    if (!PAGE_PRESENT(l4_entry)) {
        uk_pr_info("No entry 0x%08lx in L4 table for address 0x%08lx\n", l4_entry, vaddr);

        return -1;
    }

    unsigned long l3_table = PTE_REMOVE_FLAGS(l4_entry);
    unsigned long l3_entry = get_l3_entry(l3_table, L3_OFFSET(vaddr));
    uk_pr_info("L3 entry for addr 0x%08lx is 0x%08lx\n", vaddr, l3_entry);
    if (!PAGE_PRESENT(l3_entry)) {
        uk_pr_info("No entry 0x%08lx in L3 table for address 0x%08lx\n", l3_entry, vaddr);

        return -1;
    }
    if (PAGE_HUGE(l3_entry)) {
        uk_pr_info("Entry for 0x%08lx is mapped in a huge page (1G) with L3 entry 0x%08lx\n", vaddr, l3_entry);
    }

    unsigned long l2_table = PTE_REMOVE_FLAGS(l3_entry);
    unsigned long l2_entry = get_l2_entry(l2_table, L2_OFFSET(vaddr));
    uk_pr_info("L2 entry for addr 0x%08lx is 0x%08lx\n", vaddr, l2_entry);
    if (!PAGE_PRESENT(l2_entry)) {
        uk_pr_info("No entry 0x%08lx in L2 table for address 0x%08lx\n", l2_entry, vaddr);

        return -1;
    }
    if (PAGE_LARGE(l2_entry)) {
        uk_pr_info("Entry for 0x%08lx is mapped in a large page (2M) with L2 entry 0x%08lx\n", vaddr, l2_entry);
    }

    unsigned long l1_table = PTE_REMOVE_FLAGS(l2_entry);
    unsigned long l1_entry = get_l1_entry(l1_table, L1_OFFSET(vaddr));
    if (!PAGE_PRESENT(l1_entry))
        return -1;

    unsigned long new_l1_entry = create_l1_entry(PTE_REMOVE_FLAGS(l1_entry), new_prot);
    write_l1_entry(l1_table, L1_OFFSET(vaddr), new_l1_entry);
    ukarch_flush_tlb_entry(vaddr);

    return 0;
}

// return 1 if |vaddr| does not point to an already mapped page
// return 0 otherwise
int can_map(unsigned long vaddr)
{
    if (!PAGE_ALIGNED(vaddr)) {
        uk_pr_info("Address must be aligned to page size\n");
        return 0;
    }

    unsigned long l4_table = PTE_REMOVE_FLAGS(ukarch_read_pt_base());

    unsigned long l4_entry = get_l4_entry(l4_table, L4_OFFSET(vaddr));
    if (!PAGE_PRESENT(l4_entry))
        return 1;

    unsigned long l3_table = PTE_REMOVE_FLAGS(l4_entry);
    unsigned long l3_entry = get_l3_entry(l3_table, L3_OFFSET(vaddr));
    if (!PAGE_PRESENT(l3_entry))
        return 1;
    if (PAGE_HUGE(l3_entry))
        return 0;

    unsigned long l2_table = PTE_REMOVE_FLAGS(l3_entry);
    unsigned long l2_entry = get_l2_entry(l2_table, L2_OFFSET(vaddr));
    if (!PAGE_PRESENT(l2_entry))
        return 1;
    if (PAGE_LARGE(l2_entry))
        return 0;

    unsigned long l1_table = PTE_REMOVE_FLAGS(l2_entry);
    unsigned long l1_entry = get_l1_entry(l1_table, L1_OFFSET(vaddr));

    return !PAGE_PRESENT(l1_entry);
}

int uk_pt_init(unsigned long paddr_start, size_t len)
{
    paddr_start = PAGE_ALIGN(paddr_start);
    bitmap_start_addr = paddr_start;
    bitmap_length = len / PAGE_SIZE;
    pt_start_addr = PAGE_ALIGN(paddr_start + bitmap_length);
    pt_length = PAGE_ALIGN_DOWN(len - bitmap_length);

    uk_pr_err("START is 0x%016llx and LEN is 0x%016llx\n", paddr_start, len);

    uk_bitmap_zero(bitmap_start_addr, bitmap_length);
    memset(pt_start_addr, 0, pt_length);
}
