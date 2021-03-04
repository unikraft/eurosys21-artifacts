/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Authors: Simon Kuenzer <simon.kuenzer@neclab.eu>
 *
 *
 * Copyright (c) 2017, NEC Europe Ltd., NEC Corporation. All rights reserved.
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
#include <uk/arch/types.h>
#include <linuxu/setup.h>
#include <uk/errptr.h>
#include <uk/assert.h>
#include <linuxu/syscall.h>
#include <uk/plat/memory.h>
#include <uk/libparam.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define MB2B (1024 * 1024)

static __u32 heap_size = CONFIG_LINUXU_DEFAULT_HEAPMB;
UK_LIB_PARAM(heap_size, __u32);

static const char *initrd_file = NULL;
UK_LIB_PARAM_STR(initrd_file);

static int __linuxu_plat_heap_init(void)
{
	void *pret;
	int rc = 0;

	_liblinuxuplat_opts.heap.len = heap_size * MB2B;
	uk_pr_info("Allocate memory for heap (%u MiB)\n", heap_size);

	/**
	 * Allocate heap memory
	 */
	if (_liblinuxuplat_opts.heap.len > 0) {
		pret = sys_mapmem(NULL, _liblinuxuplat_opts.heap.len);
		if (PTRISERR(pret)) {
			rc = PTR2ERR(pret);
			uk_pr_err("Failed to allocate memory for heap: %d\n",
				  rc);
		} else
			_liblinuxuplat_opts.heap.base = pret;
	}

	return rc;
}

static int __linuxu_plat_initrd_init(void)
{
	void *pret;
	int rc = 0;
	struct stat file_info;

	if (initrd_file == NULL) {
		uk_pr_debug("No initrd present.\n");
	} else {
		uk_pr_debug("Mapping in initrd file: %s\n", initrd_file);
		int initrd_fd = sys_open(initrd_file, O_RDONLY, 0);

		if (initrd_fd < 0) {
			uk_pr_err("Failed to open intrd file");
			return -1;
		}

		/**
		 * Find initrd file size
		 */
		if (sys_fstat(initrd_fd, &file_info) < 0) {
			uk_pr_err("sys_fstat failed for initrd file");
			close(initrd_fd);
			return -1;
		}
		_liblinuxuplat_opts.initrd.len = file_info.st_size;
		/**
		 * Allocate initrd memory
		 */
		if (_liblinuxuplat_opts.initrd.len > 0) {
			pret = sys_mmap((void *)_liblinuxuplat_opts.heap.len,
					_liblinuxuplat_opts.initrd.len,
					PROT_READ | PROT_WRITE | PROT_EXEC,
					MAP_PRIVATE, initrd_fd, 0);
			if (PTRISERR(pret)) {
				rc = PTR2ERR(pret);
				uk_pr_err("Failed to allocate memory for initrd: %d\n",
					  rc);
				close(initrd_fd);
				return -1;
			}
			_liblinuxuplat_opts.initrd.base = pret;
		} else {
			uk_pr_err("Empty initrd file given.\n");
			close(initrd_fd);
			return -1;
		}
	}
	return rc;
}

int ukplat_memregion_count(void)
{
	static int have_heap = 0;
	static int have_initrd = 0;
	int rc = 0;

	/*
	 * NOTE: The heap size and initrd file can be changed by a
	 * library parameter. We assume that those ones are processed
	 * by the boot library shortly before memory regions are
	 * scanned. This is why we initialize the heap here.
	 */
	if (!have_heap) {
		rc = __linuxu_plat_heap_init();
		have_heap = (rc == 0) ? 1 : 0;
	}
	if (!have_initrd) {
		rc = __linuxu_plat_initrd_init();
		have_initrd = (rc == 0) ? 1 : 0;
	}
	return have_heap + have_initrd;
}

int ukplat_memregion_get(int i, struct ukplat_memregion_desc *m)
{
	int ret;

	UK_ASSERT(m);

	if (i == 0 && _liblinuxuplat_opts.heap.base) {
		m->base = _liblinuxuplat_opts.heap.base;
		m->len = _liblinuxuplat_opts.heap.len;
		m->flags = UKPLAT_MEMRF_ALLOCATABLE;
#if CONFIG_UKPLAT_MEMRNAME
		m->name = "heap";
#endif
		ret = 0;
	} else if ((i == 0 && !_liblinuxuplat_opts.heap.base
		    && _liblinuxuplat_opts.initrd.base)
		   || (i == 1 && _liblinuxuplat_opts.heap.base
		       && _liblinuxuplat_opts.initrd.base)) {
		m->base = _liblinuxuplat_opts.initrd.base;
		m->len = _liblinuxuplat_opts.initrd.len;
		m->flags = UKPLAT_MEMRF_INITRD | UKPLAT_MEMRF_WRITABLE;
#if CONFIG_UKPLAT_MEMRNAME
		m->name = "initrd";
#endif
		ret = 0;
	} else {
		/* invalid memory region index or no heap allocated */
		m->base = __NULL;
		m->len = 0;
		m->flags = 0x0;
#if CONFIG_UKPLAT_MEMRNAME
		m->name = __NULL;
#endif
		ret = -1;
	}

	return ret;
}

int _ukplat_mem_mappings_init(void)
{
	return 0;
}

void ukplat_stack_set_current_thread(void *thread_addr __unused)
{
	/* For now, signals use the current process stack */
}
