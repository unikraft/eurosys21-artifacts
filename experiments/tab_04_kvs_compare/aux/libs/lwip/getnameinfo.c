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
#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

int getnameinfo(const struct sockaddr *restrict sa, socklen_t sl,
	char *restrict node, socklen_t nodelen,
	char *restrict serv, socklen_t servlen,
	int flags)
{
	char buf[256];
	/*unsigned char reply[512]; TODO used in DNS reply */
	int af = sa->sa_family;
#if CONFIG_LIBNEWLIBC /* because of fopen() */
	char line[512];
	FILE *f;
#endif
	unsigned char *a;

	switch (af) {
	case AF_INET:
		a = (void *) &((struct sockaddr_in *) sa)->sin_addr;
		if (sl != sizeof(struct sockaddr_in))
			return EAI_FAMILY;
		break;
#if CONFIG_LWIP_IPV6
	case AF_INET6:
		a = (void *) &((struct sockaddr_in6 *) sa)->sin6_addr;
		if (sl != sizeof(struct sockaddr_in6))
			return EAI_FAMILY;
		break;
#endif
	default:
		return EAI_FAMILY;
	}

#if CONFIG_LIBNEWLIBC /* because of fopen() */
	/* Try to find ip within /etc/hosts */
	if ((node && nodelen) && (af == AF_INET)) {
		const char *ipstr;
		size_t l;

		ipstr = inet_ntoa(((struct sockaddr_in *)sa)->sin_addr);
		l = strlen(ipstr);
		f = fopen("/etc/hosts", "r");
		if (f)
			while (fgets(line, sizeof(line), f)) {
				char *domain;

				if (strncmp(line, ipstr, l) != 0)
					continue;

				domain = strtok(line, " ");
				if (!domain)
					continue;
				domain = strtok(NULL, " ");
				if (!domain)
					continue;

				if (strlen(domain) >= nodelen)
					return EAI_OVERFLOW;
				strcpy(node, domain);
				fclose(f);
				return 0;
			}
		if (f)
			fclose(f);
	}
#endif

	if (node && nodelen) {
		if ((flags & NI_NUMERICHOST)
#if 0
			/* TODO we currently don't support name requests */
			|| __dns_query(reply, a, af, 1) <= 0
			|| __dns_get_rr(buf, 0, 256, 1, reply, RR_PTR, 1) <= 0) {
#else
			|| 1) {
#endif
			if (flags & NI_NAMEREQD)
				return EAI_NONAME;
			inet_ntop(af, a, buf, sizeof(buf));
		}
		if (strlen(buf) >= nodelen)
			return EAI_OVERFLOW;
		strcpy(node, buf);
	}

	if (serv && servlen) {
		if (snprintf(buf, sizeof(buf), "%d",
			ntohs(((struct sockaddr_in *) sa)->sin_port)) >= (int) servlen)
			return EAI_OVERFLOW;
		strcpy(serv, buf);
	}

	return 0;
}
