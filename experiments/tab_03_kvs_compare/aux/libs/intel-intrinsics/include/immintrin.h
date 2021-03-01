/*===---- immintrin.h - Intel intrinsics -----------------------------------===
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *===-----------------------------------------------------------------------===
 */

#ifndef __IMMINTRIN_H
#define __IMMINTRIN_H

#if defined(__MMX__)
#include <mmintrin.h>
#endif

#if defined(__SSE__)
#include <xmmintrin.h>
#endif

#if defined(__SSE2__)
#include <emmintrin.h>
#endif

#if defined(__SSE3__)
#include <pmmintrin.h>
#endif

#if defined(__SSSE3__)
#include <tmmintrin.h>
#endif

#if \
    (defined(__SSE4_2__) || defined(__SSE4_1__))
#include <smmintrin.h>
#endif

#if defined(__AVX__)
#include <avxintrin.h>
#endif

#if defined(__POPCNT__)
#include <popcntintrin.h>
#endif


/* __bit_scan_forward */
/*
static __inline__ int __attribute__((__always_inline__, __nodebug__))
_bit_scan_forward(int __A) {
  return __builtin_ctz(__A);
}
*/
/* __bit_scan_reverse */
/*
static __inline__ int __attribute__((__always_inline__, __nodebug__))
_bit_scan_reverse(int __A) {
  return 31 - __builtin_clz(__A);
}
*/
#endif /* __IMMINTRIN_H */
