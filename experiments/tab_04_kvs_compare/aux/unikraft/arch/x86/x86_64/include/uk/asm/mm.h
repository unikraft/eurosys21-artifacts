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
 *
 * Some of these macros here were inspired from Xen code.
 * For example, from "xen/include/asm-x86/x86_64/page.h" file.
 */

#ifndef __UKPLAT_X86_64_MM__
#define __UKPLAT_X86_64_MM__

#define L1_PAGETABLE_SHIFT      12
#define L2_PAGETABLE_SHIFT      21
#define L3_PAGETABLE_SHIFT      30
#define L4_PAGETABLE_SHIFT      39

#define L1_PAGETABLE_ENTRIES    512
#define L2_PAGETABLE_ENTRIES    512
#define L3_PAGETABLE_ENTRIES    512
#define L4_PAGETABLE_ENTRIES    512

#define L1_OFFSET(vaddr) (((vaddr) >> L1_PAGETABLE_SHIFT) & (L1_PAGETABLE_ENTRIES - 1))
#define L2_OFFSET(vaddr) (((vaddr) >> L2_PAGETABLE_SHIFT) & (L2_PAGETABLE_ENTRIES - 1))
#define L3_OFFSET(vaddr) (((vaddr) >> L3_PAGETABLE_SHIFT) & (L3_PAGETABLE_ENTRIES - 1))
#define L4_OFFSET(vaddr) (((vaddr) >> L4_PAGETABLE_SHIFT) & (L4_PAGETABLE_ENTRIES - 1))

#define _PAGE_PRESENT  0x001UL
#define _PAGE_RW       0x002UL
#define _PAGE_USER     0x004UL
#define _PAGE_PWT      0x008UL
#define _PAGE_PCD      0x010UL
#define _PAGE_ACCESSED 0x020UL
#define _PAGE_DIRTY    0x040UL
#define _PAGE_PAT      0x080UL
#define _PAGE_PSE      0x080UL
#define _PAGE_GLOBAL   0x100UL
#define _PAGE_NX       (1UL << 63)

#define PAGE_PRESENT(vaddr) ((vaddr) & _PAGE_PRESENT)
#define PAGE_LARGE(vaddr)   ((vaddr) & _PAGE_PSE)
#define PAGE_HUGE(vaddr)    ((vaddr) & _PAGE_PSE)

#define L1_PROT    (_PAGE_PRESENT | _PAGE_RW | _PAGE_ACCESSED)
#define L1_PROT_RO (_PAGE_PRESENT | _PAGE_ACCESSED)
#define L2_PROT    (_PAGE_PRESENT | _PAGE_RW | _PAGE_ACCESSED | _PAGE_DIRTY)
#define L3_PROT    (_PAGE_PRESENT | _PAGE_RW | _PAGE_ACCESSED | _PAGE_DIRTY)
#define L4_PROT    (_PAGE_PRESENT | _PAGE_RW | _PAGE_ACCESSED | _PAGE_DIRTY)

/* round down to nearest page address */
#define PAGE_ALIGN_DOWN(vaddr) ((vaddr) & ~(PAGE_SIZE - 1))
#define PTE_REMOVE_FLAGS(vaddr) PAGE_ALIGN_DOWN(vaddr & ~_PAGE_NX)

/* round up to nearest page address */
#define PAGE_ALIGN(vaddr) (((vaddr) + (PAGE_SIZE - 1)) & ~(PAGE_SIZE - 1))

/* returns 1 if |vaddr| is page aligned */
#define PAGE_ALIGNED(vaddr) (!((vaddr) & (PAGE_SIZE - 1)))


static inline unsigned long ukarch_read_pt_base()
{
    unsigned long cr3;

    __asm__ __volatile__("movq %%cr3, %0" :"=r"(cr3)::);

    return cr3;
}

static inline unsigned long create_l1_entry(unsigned long paddr, unsigned long prot)
{
    unsigned long flags;

    if (prot & PAGE_PROT_WRITE)
        flags = L1_PROT;
    else
        flags = L1_PROT_RO;

    if (!(prot & PAGE_PROT_EXEC))
        flags |= _PAGE_NX;

    return paddr | flags;
}

static inline void ukarch_flush_tlb_entry(unsigned long vaddr)
{
    __asm__ __volatile__("invlpg (%0)" ::"r" (vaddr) : "memory");
}


#endif  /* __UKPLAT_X86_64_MM__ */
