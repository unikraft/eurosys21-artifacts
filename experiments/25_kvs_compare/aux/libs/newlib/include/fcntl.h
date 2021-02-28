#ifndef _NEWLIB_GLUE_FCNTL_H_
#define _NEWLIB_GLUE_FCNTL_H_

#include_next <fcntl.h>
#define loff_t off_t

#include <uk/config.h>

#if (defined CONFIG_ARCH_X86_64)
#define O_NOFOLLOW  0400000
#define O_DIRECTORY 0200000
#define O_CLOEXEC  02000000
#define O_DSYNC      010000
#define O_ASYNC      020000
#define O_DIRECT     040000
#define O_NOATIME  01000000
#elif ((defined CONFIG_ARCH_ARM_64) || (defined CONFIG_ARCH_ARM_32))
#define O_NOFOLLOW  0100000
#define O_DIRECTORY  040000
#define O_CLOEXEC  02000000
#define O_DSYNC      010000
#define O_ASYNC      020000
#define O_DIRECT    0200000
#define O_NOATIME  01000000
#endif

#define FIONBIO      0x5421
#define FIOASYNC     0x5452

#define POSIX_FADV_NORMAL     0
#define POSIX_FADV_RANDOM     1
#define POSIX_FADV_SEQUENTIAL 2
#define POSIX_FADV_WILLNEED   3
#define POSIX_FADV_DONTNEED   4
#define POSIX_FADV_NOREUSE    5

/* Glibc does not provide KEEP_SIZE and PUNCH_HOLE anymore. Instead it
 * includes linux/falloc.h.
 *
 * Musl still does provide them. And Newlib just does not care.
 */
#define FALLOC_FL_KEEP_SIZE 1
#define FALLOC_FL_PUNCH_HOLE 2

#if defined(_GNU_SOURCE) || defined(_BSD_SOURCE)
#define AT_EMPTY_PATH 0x1000
#endif

int posix_fadvise(int fd, off_t offset, off_t len, int advice);

#endif
