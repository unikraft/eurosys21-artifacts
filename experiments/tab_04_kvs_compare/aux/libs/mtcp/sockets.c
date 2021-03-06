/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Authors: Alexander Jung <alexander.jung@neclab.eu>
 *
 * Copyright (c) 2020, NEC Laboratories Europe GmbH, NEC Corporation.
 *                     All rights reserved.
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
 */

#include "mtcp_config.h"

#include <sys/socket.h>
#include <uk/socket.h>
#include <uk/alloc.h>
#include <mtcp_api.h>
#include <uk/print.h>

#define MTCP_CPU_ID 0

struct mtcp_socket {
  /* Context object used internally by mTCP. */
  mctx_t mctx;
  /* File descriptor of the mTCP socket. */
  int mtcp_fd;
};

int
mtcp_lib_init(struct posix_socket_driver *d)
{
  uk_pr_debug("mtcp_lib_init\n");

  return 0;
}

int
mtcp_glue_create(struct posix_socket_driver *d,
          int family, int type, int protocol)
{
  void *ret = NULL;
  int vfs_fd = 0;
  struct mtcp_socket *mtcp_sock;
  
  /* Use our socket data store to hold onto mTCP's context object. */
  mtcp_sock = uk_calloc(d->allocator, 1, sizeof(struct mtcp_socket));
  if (!mtcp_sock) {
    ret = -ENFILE;
    SOCKET_LIB_ERR(d, ret, "could not allocate socket: out of memory");
    goto EXIT;
  }

  mtcp_sock->mctx = mtcp_create_context(MTCP_CPU_ID);
  if (mtcp_sock->mctx == NULL) {
    ret = NULL;
    goto MTCP_SOCKET_CLEANUP;
  }

  /* Create an mTCP socket */
  mtcp_sock->mtcp_fd = mtcp_socket(mtcp_sock->mctx, family, type, protocol);
  if (mtcp_sock->mtcp_fd < 0) {
    ret = NULL;
    goto MTCP_SOCKET_CLEANUP; 
  }

  /* Return the whole mtcp socket struct for the driver */
  ret = mtcp_sock;
  
EXIT:
  return ret;

MTCP_SOCKET_CLEANUP:
  uk_free(d->allocator, mtcp_sock);
  goto EXIT;
}

static int
mtcp_glue_accept(struct posix_socket_driver *d,
          void *sock, struct sockaddr *addr,
          socklen_t *addr_len)
{
  uk_pr_debug("mtcp_glue_accept\n");
  return 0;
}

static int
mtcp_glue_bind(struct posix_socket_driver *d,
          void *sock, struct sockaddr *addr,
          socklen_t addr_len)
{
  uk_pr_debug("mtcp_glue_bind\n");

  int ret = 0;
  struct mtcp_socket *mtcp_sock;

  /* Transform the socket descriptor to the mtcp_socket pointer. */
  mtcp_sock = (struct mtcp_socket *)sock;
  
  /* Create an LwIP socket */
  ret = mtcp_bind(mtcp_sock->mctx, mtcp_sock->mtcp_fd, addr, addr_len);
  if (ret < 0) {
    ret = -1;
  }

  return ret;
}

static int
mtcp_glue_shutdown(struct posix_socket_driver *d,
          void *sock, int how)
{
  return 0;
}

static int
mtcp_glue_getpeername(struct posix_socket_driver *d,
          void *sock, struct sockaddr *addr,
          socklen_t addr_len)
{
  return 0;
}

static int
mtcp_glue_getsockname(struct posix_socket_driver *d,
          void *sock, struct sockaddr *addr,
          socklen_t addr_len)
{
  return 0;
}

static int
mtcp_glue_getsockopt(struct posix_socket_driver *d,
          void *sock, int level, int optname, void *restrict optval,
          socklen_t *restrict optlen)
{
  return 0;
}

static int
mtcp_glue_setsockopt(struct posix_socket_driver *d,
          void *sock, int level, int optname, const void *optval,
          socklen_t optlen)
{
  return 0;
}

static int
mtcp_glue_connect(struct posix_socket_driver *d,
          void *sock, struct sockaddr *addr,
          socklen_t addr_len)
{
  return 0;
}

static int
mtcp_glue_listen(struct posix_socket_driver *d,
          void *sock, int backlog)
{
  return 0;
}

static ssize_t
mtcp_glue_recv(struct posix_socket_driver *d,
          void *sock, void *buf, size_t len, int flags)
{
  return 0;
}

static ssize_t
mtcp_glue_recvfrom(struct posix_socket_driver *d,
          void *sock, void *restrict buf, size_t len, int flags,
          struct sockaddr *from, socklen_t *restrict fromlen)
{
  return 0;
}

static ssize_t
mtcp_glue_recvmsg(struct posix_socket_driver *d,
          void *sock, struct posix_socket_msghdr *msg, int flags)
{
  return 0;
}

static ssize_t
mtcp_glue_send(struct posix_socket_driver *d,
          void *sock, const void *buf, size_t len, int flags)
{
  return 0;
}

static ssize_t
mtcp_glue_sendmsg(struct posix_socket_driver *d,
          void *sock, const void *buf, size_t len, int flags,
          const struct sockaddr *dest_addr, socklen_t addrlen)
{
  return 0;
}

static ssize_t
mtcp_glue_sendto(struct posix_socket_driver *d,
          void *sock, struct posix_socket_msghdr *msg, int flags)
{
  return 0;
}

static int
mtcp_glue_socketpair(struct posix_socket_driver *d,
          int family, int type, int protocol, int *usockvec)
{
  return 0;
}

static int
mtcp_glue_read(struct posix_socket_driver *d,
          void *sock, void *buf, size_t count)
{
  return 0;
}

static int
mtcp_glue_close(struct posix_socket_driver *d,
          void *sock)
{
  return 0;
}

static struct posix_socket_ops mtcp_socket_ops = {
  /* The initialization function on socket registration. */
  .init        = mtcp_lib_init,
  /* POSIX interfaces */
  .create      = mtcp_glue_create,
  .accept      = mtcp_glue_accept,
  .bind        = mtcp_glue_bind,
  .shutdown    = mtcp_glue_shutdown,
  .getpeername = mtcp_glue_getpeername,
  .getsockname = mtcp_glue_getsockname,
  .getsockopt  = mtcp_glue_getsockopt,
  .setsockopt  = mtcp_glue_setsockopt,
  .connect     = mtcp_glue_connect,
  .listen      = mtcp_glue_listen,
  .recv        = mtcp_glue_recv,
  .recvfrom    = mtcp_glue_recvfrom,
  .recvmsg     = mtcp_glue_recvmsg,
  .send        = mtcp_glue_send,
  .sendmsg     = mtcp_glue_sendmsg,
  .sendto      = mtcp_glue_sendto,
  /* vfscore ops */
  .read        = mtcp_glue_read,
  // .write       = mtcp_glue_write,
  .close       = mtcp_glue_close,
  // .ioctl       = mtcp_glue_ioctl,
};

POSIX_SOCKET_FAMILY_REGISTER(AF_INET, &mtcp_socket_ops, NULL);
// POSIX_SOCKET_FAMILY_REGISTER(AF_INET6, &mtcp_socket_ops, NULL);
