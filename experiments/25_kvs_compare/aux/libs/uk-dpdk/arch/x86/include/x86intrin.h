/*===---- x86intrin.h - X86 intrinsics -------------------------------------===
 *
 * Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
 * See https://llvm.org/LICENSE.txt for license information.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 *
 *===-----------------------------------------------------------------------===
 */

#ifndef __X86INTRIN_H
#define __X86INTRIN_H

#if 0
#endif /* DISABLE ia32intrin.h */

#include <uk/compile.h>
#include <x86intrin.h>
#include <emmintrin.h>
#include <immintrin.h>
#include <wmmintrin.h>

#if 0
#if !defined(_MSC_VER) || __has_feature(modules) || defined(__3dNOW__)
#include <mm3dnow.h>
#endif

#if !defined(_MSC_VER) || __has_feature(modules) || defined(__PRFCHW__)
#include <prfchwintrin.h>
#endif

#if !defined(_MSC_VER) || __has_feature(modules) || defined(__SSE4A__)
#include <ammintrin.h>
#endif

#if !defined(_MSC_VER) || __has_feature(modules) || defined(__FMA4__)
#include <fma4intrin.h>
#endif

#if !defined(_MSC_VER) || __has_feature(modules) || defined(__XOP__)
#include <xopintrin.h>
#endif

#if !defined(_MSC_VER) || __has_feature(modules) || defined(__TBM__)
#include <tbmintrin.h>
#endif

#if !defined(_MSC_VER) || __has_feature(modules) || defined(__LWP__)
#include <lwpintrin.h>
#endif

#if !defined(_MSC_VER) || __has_feature(modules) || defined(__MWAITX__)
#include <mwaitxintrin.h>
#endif

#if !defined(_MSC_VER) || __has_feature(modules) || defined(__CLZERO__)
#include <clzerointrin.h>
#endif
#endif /* DISABLE_INTRINSICS */


#endif /* __X86INTRIN_H */
