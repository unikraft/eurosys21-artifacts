/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Authors: Bogdan Lascu <lascu.bogdan96@gmail.com>
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

#include <sys/socket.h>
#include <netdb.h>


const char *inet_ntop(int af, const void *src, char *dst, socklen_t size)
{
	return lwip_inet_ntop(af, src, dst, size);
}

int inet_pton(int af, const char *src, void *dst)
{
	return lwip_inet_pton(af, src, dst);
}

#if LWIP_DNS && LWIP_SOCKET && !(LWIP_COMPAT_SOCKETS)
int getaddrinfo(const char *node, const char *service,
		const struct addrinfo *hints,
		struct addrinfo **res)
{
	return lwip_getaddrinfo(node, service, hints, res);
}

void freeaddrinfo(struct addrinfo *res)
{
	return lwip_freeaddrinfo(res);
}
#endif /* LWIP_DNS && LWIP_SOCKET && !(LWIP_COMPAT_SOCKETS) */

/* Note: lwip implementation of getaddrinfo does not return all the errors
 * codes mentioned in its man page.
 */
const char *gai_strerror(int errcode)
{
	switch (errcode) {
#if LWIP_DNS_API_DEFINE_ERRORS
	case EAI_NONAME:
		return "The node or service is not known; or both node and service are NULL.";
	case EAI_SERVICE:
		return "The requested service is not available for the requested socket type.";
	case EAI_FAIL:
		return "The name server returned a permanent failure indication.";
	case EAI_MEMORY:
		return "Out of memory.";
	case EAI_FAMILY:
		return "The requested address family is not supported.";
	case EAI_OVERFLOW:
		return "The buffer pointed to by host or serv was too small.";
#endif /* LWIP_DNS_API_DEFINE_ERRORS */
	default:
		return "Error on getaddrinfo.";
	}
}
