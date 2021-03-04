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

#include <uk/bitmap.h>
#include <uk/print.h>
#include <uk/plat/mm.h>

#include <mm.h>

/*
 * XXX
 * This does for now a linear search, starting from |start|, looking for a
 * memory area with |length| bytes (aligned to page size)
 */
static unsigned long get_free_virtual_area(unsigned long start, size_t length,
        unsigned long prot)
{
    if (length & (PAGE_SIZE - 1))
        return -1;

    unsigned long page;

    /* TODO: stop condition */
    while (start >= MMAP_MIN_ADDR) {
        for (page = start; page < start + length; page += PAGE_SIZE) {
            if (!can_map(page))
                break;
        }

        if (page == start + length)
            return start;
        else
            start = page + PAGE_SIZE;
    }

    return -1;
}

void *mmap(void *addr, size_t length, int prot, int flags,
        int fd, off_t offset)
{
    /* We don't currently support mapping files */
    if (!(flags & MAP_ANONYMOUS)) {
        errno = ENOTSUP;
        return MAP_FAILED;
    }

    if (fd != -1 || offset) {
        errno = EINVAL;
        return MAP_FAILED;
    }

    /* At least one of MAP_SHARED or MAP_PRIVATE has to be specified */
    if (!(flags & MAP_SHARED) && !(flags & MAP_PRIVATE)) {
        errno = EINVAL;
        return MAP_FAILED;
    }

    if ((prot & PROT_NONE) && (prot != PROT_NONE)) {
        errno = EINVAL;
        return MAP_FAILED;
    }

    if (!length) {
        errno = EINVAL;
        return MAP_FAILED;
    }

    length = PAGE_ALIGN(length);
    if (!length) {
        errno = ENOMEM;
        return MAP_FAILED;
    }

    if (addr != NULL)
        addr = (void *) PAGE_ALIGN((unsigned long) addr);

    /* TODO: This might not be the best way */
    if (addr == NULL || (unsigned long) addr < MMAP_MIN_ADDR)
        addr = MMAP_MIN_ADDR;

    unsigned long where = get_free_virtual_area(addr, length, prot);
    if (where == -1) {
        errno = ENOMEM;
        return MAP_FAILED;
    }

    unsigned long page_prot = 0;
    if (prot & PROT_READ)
        page_prot |= PAGE_PROT_READ;
    if (prot & PROT_WRITE)
        page_prot |= PAGE_PROT_WRITE;
    if (prot & PROT_EXEC)
        page_prot |= PAGE_PROT_EXEC;

    for (unsigned long page = where; page < where + length; page += PAGE_SIZE)
        /* TODO change physical address (2nd param) */
        uk_page_map(page, page, PAGE_PROT_READ | PAGE_PROT_WRITE);

    /* Only true for MAP_ANONYMOUS */
    memset(where, 0, length);

    for (unsigned long page = where; page < where + length; page += PAGE_SIZE)
        uk_page_change_prot(page, page_prot);

    return where;
}

int munmap(void *addr, size_t len)
{
    unsigned long start = (unsigned long) addr;
    if (start & (PAGE_SIZE - 1)) {
        errno = EINVAL;
        return -1;
    }

    if (!len)
        return 0;

    len = PAGE_ALIGN(len);
    for (unsigned long page = start; page < start + len; page += PAGE_SIZE) {
        uk_page_unmap(page);
    }

    return 0;
}

int mprotect(void *addr, size_t len, int prot)
{
    unsigned long start = (unsigned long) addr;
    if (start & (PAGE_SIZE - 1)) {
        errno = EINVAL;
        return -1;
    }

    if (!len)
        return 0;

    unsigned long page_prot = 0;
    if (prot & PROT_READ)
        page_prot |= PAGE_PROT_READ;
    if (prot & PROT_WRITE)
        page_prot |= PAGE_PROT_WRITE;
    if (prot & PROT_EXEC)
        page_prot |= PAGE_PROT_EXEC;

    len = PAGE_ALIGN(len);
    for (unsigned long page = start; page < start + len; page += PAGE_SIZE) {
        uk_page_change_prot(page, page_prot);
    }

    return 0;
}

int msync(void *addr, size_t length, int flags)
{
    errno = ENOTSUP;
    return -1;
}

/**
 *   *  Stub implementation
 *     */
  int mlock(const void *addr, size_t len)
  {
      return 0;
  }

