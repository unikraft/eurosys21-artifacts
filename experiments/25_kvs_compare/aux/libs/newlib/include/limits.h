/* SPDX-License-Identifier: MIT */
/* ----------------------------------------------------------------------
 * Copyright © 2005-2014 Rich Felker, et al.
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
/* Taken from musl */

#ifndef _POSIX_LIMITS_H
#define _POSIX_LIMITS_H

#include <uk/arch/limits.h>

#define CHAR_BIT        8

#define SCHAR_MAX       0x7f
#define SCHAR_MIN       (-SCHAR_MAX-1)
#define UCHAR_MAX       0xff

#ifdef __CHAR_UNSIGNED__
# define CHAR_MIN       0
# define CHAR_MAX       UCHAR_MAX
#else
# define CHAR_MIN       SCHAR_MIN
# define CHAR_MAX       SCHAR_MAX
#endif

#define INT_MAX         0x7fffffff
#define INT_MIN         (-INT_MAX-1)
#define UINT_MAX        0xffffffff

#define SHRT_MIN        (-0x8000)
#define SHRT_MAX        0x7fff
#define USHRT_MAX       0xffff

#if defined(__x86_64__)
# define LONG_MAX       0x7fffffffffffffffL
# define ULONG_MAX      0xffffffffffffffffUL
#else
# define LONG_MAX       0x7fffffffL
# define ULONG_MAX      0xffffffffUL
#endif
#define LONG_MIN        (-LONG_MAX-1L)
#define LLONG_MAX       0x7fffffffffffffffLL
#define LLONG_MIN       (-LLONG_MAX-1LL)
#define ULLONG_MAX      0xffffffffffffffffULL

#define LONG_LONG_MIN   LLONG_MIN
#define LONG_LONG_MAX   LLONG_MAX
#define ULONG_LONG_MAX  ULLONG_MAX
#define SSIZE_MAX       LONG_MAX

#ifndef MB_LEN_MAX
#define MB_LEN_MAX      1
#endif

#define PATH_MAX        1024

/* The maximum number of repeated occurrences of a regular expression
 * permitted when using the interval notation `\{M,N\}'.
 */
#define	_POSIX2_RE_DUP_MAX      255
#endif /* _POSIX_LIMITS_H */
