/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Authors: Vlad-Andrei Badoiu <vlad_andrei.badoiu@stud.acs.upb.ro
 *
 * Copyright (c) 2019, University Politehnica of Bucharest. All rights reserved.
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

#include <uk/process.h>
#include <uk/print.h>
#include <errno.h>
#include <signal.h>

int sigaction(int sig __unused, const struct sigaction *restrict act __unused,
	      struct sigaction *restrict oact __unused)
{
	return 0;
}

unsigned int alarm(unsigned int seconds __unused)
{
	return 0;
}

int pause(void)
{
	return 0;
}

int siginterrupt(int sig __unused, int flag __unused)
{
	return 0;
}

int sigsuspend(const sigset_t *mask)
{
	return 0;
}

int kill(int pid, int sig __unused)
{
	/* TODO check sig */
	if (pid != UNIKRAFT_PID)
		errno = ESRCH;
	return -1;
}

int killpg(int pgrp, int sig __unused)
{
	/* TODO check sig */
	if (pgrp != UNIKRAFT_PGID)
		errno = ESRCH;
	return -1;
}

int sigaltstack(const stack_t *ss, stack_t *old_ss)
{
	WARN_STUBBED();
	errno = ENOTSUP;
	return -1;
}
