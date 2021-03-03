/* SPDX-License-Identifier: BSD-3-Clause AND MIT */
/*
 * Copyright (C) 2014, Cloudius Systems, Ltd.
 * Copyright (c) 2019, University Politehnica of Bucharest.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the author nor the names of any co-contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
/* For the parts taken from musl (marked as such below), the MIT licence
 * applies instead:
 * ----------------------------------------------------------------------
 * Copyright (c) 2005-2014 Rich Felker, et al.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * ----------------------------------------------------------------------
 */
// adapted from musl's version, just writes to stdio

#include <syslog.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <uk/mutex.h>


#define LOCK(x) uk_mutex_lock(&(x))
#define UNLOCK(x) uk_mutex_unlock(&(x))

static struct uk_mutex lock = UK_MUTEX_INITIALIZER(lock);
static char log_ident[32];
static int log_opt;
static int log_mask = 0xff;
static int log_facility = LOG_USER;

void openlog(const char *ident, int opt, int facility)
{
	LOCK(lock);

	if (ident) {
		size_t n;

		n = strnlen(ident, sizeof(log_ident) - 1);
		memcpy(log_ident, ident, n);
		log_ident[n] = 0;
	} else {
		log_ident[0] = 0;
	}
	log_opt = opt;
	log_facility = facility;

	UNLOCK(lock);
}

void closelog(void)
{
}

int setlogmask(int maskpri)
{
	int ret;

	LOCK(lock);
	ret = log_mask;
	if (maskpri)
		log_mask = maskpri;
	UNLOCK(lock);
	return ret;
}

void syslog(int priority, const char *message, ...)
{
	va_list ap;
	char timebuf[16];
	time_t now;
	struct tm tm;
	char buf[256];
	int pid;
	int l, l2;

	if (!(log_mask & LOG_MASK(priority & 7)) || (priority & ~0x3ff))
		return;

	LOCK(lock);

	va_start(ap, message);

	if (!(priority & LOG_FACMASK))
		priority |= log_facility;

	now = time(NULL);
	gmtime_r(&now, &tm);
	strftime(timebuf, sizeof(timebuf), "%b %e %T", &tm);

	pid = (log_opt & LOG_PID) ? getpid() : 0;
	l = snprintf(buf, sizeof(buf), "<%d>%s %s%s%.0d%s: ", priority, timebuf,
			log_ident, "[" + !pid, pid, "]" + !pid);
	l2 = vsnprintf(buf + l, sizeof(buf) - l, message, ap);
	if (l2 >= 0) {
		if (l2 >= (int) sizeof(buf) - l)
			l = sizeof(buf) - 1;
		else
			l += l2;
		if (buf[l - 1] != '\n')
			buf[l++] = '\n';
		fwrite(buf, 1, l,
			LOG_PRI(priority) >= LOG_ERR ? stderr : stdout);
	}

	va_end(ap);

	UNLOCK(lock);
}
