/*
 * lwipopts.h
 *
 * Configuration for lwIP running on mini-os
 *
 * Tim Deegan <Tim.Deegan@eu.citrix.net>, July 2007
 * Simon Kuenzer <simon.kuenzer@neclab.eu>, 2013-2019
 */
#ifndef __LWIP_LWIPOPTS_H__
#define __LWIP_LWIPOPTS_H__

#include <inttypes.h>
#include <sys/time.h>
#include <uk/config.h>

/**
 * Memory mode
 */
/* provide malloc/free by Unikraft */
#if CONFIG_LWIP_HEAP
  /* Only use malloc/free for lwIP.
   * Every allocation is done by the heap.
   * Note: This setting results in the smallest binary
   *       size but leads to heavy malloc/free usage during
   *       network processing.
   */
  #define MEM_LIBC_MALLOC 1 /* enable heap */
  #define MEMP_MEM_MALLOC 1 /* pool allocations via malloc */
#elif CONFIG_LWIP_POOLS
  /* Pools are used for pool allocations and the heap
   * is used for all the rest of allocations.
   * Note: Per design, lwIP allocates outgoing packet buffers
   *       from heap (via PBUF_RAM) and incoming from pools (via PBUF_POOL)
   *       CONFIG_LWIP_PBUF_POOL_SIZE defines the pool size for PBUF_POOL
   *       allocations
   * Note: lwIP allocate pools on the data segment
   */
  #define MEM_LIBC_MALLOC 1 /* enable heap */
  #define MEMP_MEM_MALLOC 0 /* pool allocations still via pool */
#else
 #error Configuration error!
#endif /* CONFIG_LWIP_HEAP_ONLY / CONFIG_LWIP_POOLS_ONLY */

#if MEM_LIBC_MALLOC
/*
 * alloc.c binding to the stack
 */
#include <stddef.h> /* size_t */
void *sys_malloc(size_t size);
void *sys_calloc(int num, size_t size);
void sys_free(void *ptr);

#define mem_clib_malloc   sys_malloc
#define mem_clib_calloc   sys_calloc
#define mem_clib_free     sys_free
#endif /* MEM_LIBC_MALLOC */

#if MEM_USE_POOLS
/*
 * Use lwIP's pools
 */
#define MEMP_USE_CUSTOM_POOLS 0
/* for each pool use a separate array in data segment */
#define MEMP_SEPARATE_POOLS 1
#endif /* MEM_USE_POOLS */

/**
 * Operation mode (threaded, mainloop)
 */
#if CONFIG_LWIP_NOTHREADS
#define NO_SYS 1
#else  /* CONFIG_LWIP_NOTHREADS */
#define NO_SYS 0
/* lightweight protection */
#define SYS_LIGHTWEIGHT_PROT 1
#define TCPIP_THREAD_NAME "lwip"
#define TCPIP_MBOX_SIZE 256
#endif /* CONFIG_LWIP_NOTHREADS */

/**
 * Pbuf options
 */
#define LWIP_SUPPORT_CUSTOM_PBUF 1

/**
 * Netif options
 */
#define LWIP_NETIF_REMOVE_CALLBACK 1
#define LWIP_TIMEVAL_PRIVATE 0
#define LWIP_NETIF_STATUS_CALLBACK 1

#if CONFIG_LWIP_NETIF_EXT_STATUS_CALLBACK
#define LWIP_NETIF_EXT_STATUS_CALLBACK 1
#else
#define LWIP_NETIF_EXT_STATUS_CALLBACK 0
#endif /* CONFIG_LWIP_NETIF_EXT_STATUS_CALLBACK */

#if CONFIG_LWIP_HAVE_LOOPIF
#define LWIP_HAVE_LOOPIF 1
#endif

/**
 * ARP options
 */
#define MEMP_NUM_ARP_QUEUE 256
#define ETHARP_SUPPORT_STATIC_ENTRIES 1

/**
 * IP options
 */
#if CONFIG_LWIP_IPV4
#define LWIP_IPV4 1
#else
#define LWIP_IPV4 0
#endif

#if CONFIG_LWIP_IPV6
#define LWIP_IPV6 1
#define IPV6_FRAG_COPYHEADER 1

#else
#define LWIP_IPV6 0
#endif

#if ((!LWIP_IPV4) && (!LWIP_IPV6))
#error No IP protocol was selected! Please choose at least one of LWIP_IPV4 and LWIP_IPV6
#endif

/**
 * UDP options
 */
#if CONFIG_LWIP_UDP
#define LWIP_UDP 1
#else
#define LWIP_UDP 0
#endif

/**
 * TCP options
 */
#if CONFIG_LWIP_TCP
#define LWIP_TCP 1
#else
#define LWIP_TCP 0
#endif

#if LWIP_TCP
#define TCP_MSS CONFIG_LWIP_TCP_MSS
#define TCP_CALCULATE_EFF_SEND_MSS 1
#define IP_FRAG 0

#if CONFIG_LWIP_WND_SCALE
/*
 * Maximum window and scaling factor
 */
#define LWIP_WND_SCALE  1
#if defined CONFIG_LWIP_WND_SCALE_FACTOR && CONFIG_LWIP_WND_SCALE_FACTOR >= 1
#define TCP_RCV_SCALE CONFIG_LWIP_WND_SCALE_FACTOR /* scaling factor 0..14 */
#else
#define TCP_RCV_SCALE 4
#endif /* defined CONFIG_LWIP_WND_SCALE_FACTOR && CONFIG_LWIP_WND_SCALE_FACTOR >= 1 */
#define TCP_WND 262142
#define TCP_SND_BUF ( 1024 * 1024 )

#else /* CONFIG_LWIP_WND_SCALE */
/*
 * Options when no window scaling is enabled
 */
#define LWIP_WND_SCALE 0
#define TCP_WND 32766 /* Ideally, TCP_WND should be link bandwidth multiplied by rtt */
#define TCP_SND_BUF (TCP_WND + (2 * TCP_MSS))
#endif /* CONFIG_LWIP_WND_SCALE */

#define TCP_SNDLOWAT (4 * TCP_MSS)
#define TCP_SND_QUEUELEN (2 * (TCP_SND_BUF) / (TCP_MSS))
#define TCP_QUEUE_OOSEQ 4
#define MEMP_NUM_TCP_SEG (MEMP_NUM_TCP_PCB * ((TCP_SND_QUEUELEN) / 5))
#define MEMP_NUM_FRAG_PBUF 32

#define MEMP_NUM_TCP_PCB CONFIG_LWIP_NUM_TCPCON /* max num of sim. TCP connections */
#define MEMP_NUM_TCP_PCB_LISTEN 32 /* max num of sim. TCP listeners */
#endif /* LWIP_TCP */

/**
 * Socket options
 */
#if CONFIG_LWIP_SOCKET
#define LWIP_SOCKET 1
#else
#define LWIP_SOCKET 0
#endif

#if LWIP_SOCKET
#if CONFIG_HAVE_LIBC
/* Stop lwip to provide ioctl constants */
#include <sys/ioctl.h>
#endif
/* Stop lwip to provide fd_set */
#include <sys/select.h>

#ifndef FD_SET
#define FD_SET
#endif

/* Stop lwip to provide icovec */
#include <sys/uio.h>

/*
 * We need to stop lwip introducing `struct iovec` because it is provided by
 * our libc already. The only way lwip allows us to do this is to define iovec
 * as iovec. This keeps depending definitions of lwip still working.
 */
#define iovec iovec

/*
 * disable BSD-style socket interface
 * We will provide the layer with socket.c|unikraft
 */
#define LWIP_COMPAT_SOCKETS 0

/* enable SO_REUSEADDR option */
#define SO_REUSE 1
#endif /* LWIP_SOCKET */

/**
 * DNS options
 */
#if CONFIG_LWIP_DNS
#define LWIP_DNS 1
#else
#define LWIP_DNS 0
#endif

#if LWIP_DNS
#define DNS_MAX_SERVERS CONFIG_LWIP_DNS_MAX_SERVERS
#define DNS_TABLE_SIZE CONFIG_LWIP_DNS_TABLE_SIZE
#define DNS_LOCAL_HOST_LIST 1
#define DNS_LOCAL_HOSTLIST_IS_DYNAMIC 1
#endif /* LWIP_DNS */

/**
 * Memory pool sizes
 */
#if MEM_USE_POOLS
/*
 * We dimension pool sizes on previous stack configurations so we have to do
 * this at the very last point in this configuration file.
 */
#ifndef PBUF_POOL_SIZE
#define PBUF_POOL_SIZE ((TCP_WND + TCP_MSS - 1) / TCP_MSS)
#endif
#ifndef MEMP_NUM_PBUF
#define MEMP_NUM_PBUF ((MEMP_NUM_TCP_PCB * (TCP_SND_QUEUELEN)) / 2)
#endif
#define MEMP_NUM_TCPIP_MSG_INPKT TCPIP_MBOX_SIZE
#endif /* MEM_USE_POOLS */

/**
 * Additional features
 */
#if CONFIG_LWIP_ICMP
#define LWIP_ICMP 1
#else
#define LWIP_ICMP 0
#endif

#if CONFIG_LWIP_IGMP
#define LWIP_IGMP 1
#else
#define LWIP_IGMP 0
#endif

#if CONFIG_LWIP_SNMP
#define LWIP_SNMP 1
#else
#define LWIP_SNMP 0
#endif

#if CONFIG_LWIP_DHCP
#define LWIP_DHCP 1
#else
#define LWIP_DHCP 0
#endif

#if CONFIG_LWIP_NOTHREADS
#define LWIP_NETCONN 0
#else
#define LWIP_NETCONN 1
#endif

/**
 * Checksum options
 */
/*
 * We enable the option that each netif can enable and disable
 * individually which checksums should be checked/calculated by the stack
 */
#define LWIP_CHECKSUM_CTRL_PER_NETIF 1

#define CHECKSUM_GEN_IP      1
#define CHECKSUM_GEN_IP6     1
#define CHECKSUM_GEN_ICMP    1
#define CHECKSUM_GEN_ICMP6   1
#define CHECKSUM_GEN_UDP     1
#define CHECKSUM_GEN_TCP     1
#define CHECKSUM_CHECK_IP    1
#define CHECKSUM_CHECK_UDP   1
#define CHECKSUM_CHECK_TCP   1
#define CHECKSUM_CHECK_ICMP  1
#define CHECKSUM_CHECK_ICMP6 1
#define CHECKSUM_CHECK_TCP   1

/**
 * Debugging options
 */
#if CONFIG_LWIP_DEBUG
#define LWIP_DEBUG
#define LWIP_DBG_MIN_LEVEL   LWIP_DBG_LEVEL_ALL
#define LWIP_DBG_TYPES_ON    LWIP_DBG_ON

#if CONFIG_LWIP_MAINLOOP_DEBUG
#define TCPIP_DEBUG      LWIP_DBG_ON
#define TIMERS_DEBUG     LWIP_DBG_ON
#endif /* CONFIG_LWIP_MAINLOOP_DEBUG */

#if CONFIG_LWIP_IF_DEBUG
#define ETHARP_DEBUG     LWIP_DBG_ON
#define NETIF_DEBUG      LWIP_DBG_ON
#endif /* CONFIG_LWIP_IF_DEBUG */

#if CONFIG_LWIP_IP_DEBUG
#define IP_DEBUG         LWIP_DBG_ON
#define IP6_DEBUG        LWIP_DBG_ON
#define IP_REASS_DEBUG   LWIP_DBG_ON
#endif /* CONFIG_LWIP_IP_DEBUG */

#if CONFIG_LWIP_UDP_DEBUG
#define UDP_DEBUG        LWIP_DBG_ON
#endif /* CONFIG_LWIP_UDP_DEBUG */

#if CONFIG_LWIP_TCP_DEBUG
#define TCP_DEBUG        LWIP_DBG_ON
#define TCP_FR_DEBUG     LWIP_DBG_ON
#define TCP_RTO_DEBUG    LWIP_DBG_ON
#define TCP_CWND_DEBUG   LWIP_DBG_ON
#define TCP_WND_DEBUG    LWIP_DBG_ON
#define TCP_RST_DEBUG    LWIP_DBG_ON
#define TCP_QLEN_DEBUG   LWIP_DBG_ON
#define TCP_OUTPUT_DEBUG LWIP_DBG_ON
#define TCP_INPUT_DEBUG LWIP_DBG_ON
#if LWIP_CHECKSUM_ON_COPY
#define TCP_CHECKSUM_ON_COPY_SANITY_CHECK 1
#endif
#endif /* CONFIG_LWIP_TCP_DEBUG */

#if CONFIG_LWIP_SYS_DEBUG
#define SYS_DEBUG        LWIP_DBG_ON
#define PBUF_DEBUG       LWIP_DBG_ON
#define MEM_DEBUG        LWIP_DBG_ON
#define MEMP_DEBUG       LWIP_DBG_ON
#endif /* CONFIG_LWIP_SYS_DEBUG */

#if CONFIG_LWIP_API_DEBUG
#define SOCKETS_DEBUG    LWIP_DBG_ON
#define RAW_DEBUG        LWIP_DBG_ON
#define API_MSG_DEBUG    LWIP_DBG_ON
#define API_LIB_DEBUG    LWIP_DBG_ON
#endif /* CONFIG_LWIP_API_DEBUG */

#if CONFIG_LWIP_SERVICE_DEBUG
#define ETHARP_DEBUG     LWIP_DBG_ON
#define DNS_DEBUG        LWIP_DBG_ON
#define AUTOIP_DEBUG     LWIP_DBG_ON
#define DHCP_DEBUG       LWIP_DBG_ON
#define ICMP_DEBUG       LWIP_DBG_ON
#define SNMP_DEBUG       LWIP_DBG_ON
#define SNMP_MSG_DEBUG   LWIP_DBG_ON
#define SNMP_MIB_DEBUG   LWIP_DBG_ON
#endif /* CONFIG_LWIP_SERVICE_DEBUG */
#endif /* CONFIG_LWIP_DEBUG */

#endif /* __LWIP_LWIPOPTS_H__ */
