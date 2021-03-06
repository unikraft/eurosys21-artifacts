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
#ifndef _MTCP_CONFIG_
#define _MTCP_CONFIG_

#ifndef CONFIG_MTCP_USE_PSIO
#define DISABLE_PSIO
#endif

#ifndef CONFIG_MTCP_USE_NETMAP
#define DISABLE_NETMAP
#endif

#ifndef CONFIG_MTCP_USE_ONVM
#define DISABLE_ONVM
#endif

#ifndef CONFIG_MTCP_USE_DPDK
#define DISABLE_DPDK
#endif

#include <mtcp.h>

/* Set default configuration */
static struct mtcp_config global_mtcp_config = {
  /* socket mode */
  // .socket_mode = 
  /* network interface config */
  // .eths_num =
  /* route config */
  // .routes =

  .num_cores        = 1, /* Unikraft only works with 1 core (for now) */
  #ifdef MTCP_USE_DPDK
    .num_mem_ch       = CONFIG_MTCP_NUM_MEM_CH,
  #else
    .num_mem_ch       = 0,
  #endif
  .max_concurrency  = CONFIG_MTCP_MAX_CONCURRENCY,

  .max_num_buffers  = CONFIG_MTCP_MAX_NUM_BUFFERS,
  .rcvbuf_size      = CONFIG_MTCP_RCVBUF_SIZE,
  .sndbuf_size      = CONFIG_MTCP_SNDBUF_SIZE,

  .tcp_timeout      = CONFIG_MTCP_TCP_TIMEOUT,
  .tcp_timewait     = CONFIG_MTCP_TCP_TIMEWAIT,

	/* adding multi-process support */
	// .multi_process =
	// .multi_process_is_master =

// #ifdef MTCP_USE_CCP
//   .cc               = "reno\n",
// #endif

#ifdef MTCP_ENABLE_ONVM
  /* onvm specific args */
  .onvm_inst        = (uint16_t) CONFIG_MTCP_ONVM_INST,
  .onvm_dest        = (uint16_t) CONFIG_MTCP_ONVM_DEST,
  .onvm_serv        = (uint16_t) CONFIG_MTCP_SERV_DEST,
#endif
};

#endif /* _MTCP_CONFIG_ */