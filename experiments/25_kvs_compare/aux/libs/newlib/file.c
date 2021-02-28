/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * libnewlib glue code
 *
 * Authors: Felipe Huici <felipe.huici@neclab.eu>
 *          Florian Schmidt <florian.schmidt@neclab.eu>
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

#include <uk/config.h>
#include <uk/sched.h>
#include <uk/plat/console.h>
#include <uk/print.h>
#if CONFIG_LWIP_SOCKET
#include <lwip/sockets.h>
#else
#include <poll.h>
#include <sys/select.h>
#endif
#include <sys/stat.h>
#include <errno.h>
#undef errno
#include <stdlib.h>
extern int errno;

#define STDIN_FILENO    0       /* standard input file descriptor */
#define STDOUT_FILENO   1       /* standard output file descriptor */
#define STDERR_FILENO   2       /* standard error file descriptor */

#include <sys/mman.h>

#if !CONFIG_LWIP_SOCKET
int poll(struct pollfd _pfd[] __unused, nfds_t _nfds __unused,
		int _timeout __unused)
{
	errno = ENOTSUP;
	return -1;
}

int select(int nfds, fd_set *readfds __unused, fd_set *writefds __unused,
		fd_set *exceptfds __unused, struct timeval *timeout)
{
	uint64_t nsecs;

	if (nfds == 0) {
		nsecs = timeout->tv_sec * 1000000000;
		nsecs += timeout->tv_usec * 1000;
		uk_sched_thread_sleep(nsecs);
		return 0;
	}

	errno = ENOTSUP;
	return -1;
}
#endif /* !CONFIG_LWIP_SOCKET */

int eventfd(unsigned int initval, int flags)
{
	WARN_STUBBED();
	errno = ENOTSUP;
	return -1;
}

char *realpath(const char *restrict file_name, char *restrict resolved_name)
{
	return 0;
}
