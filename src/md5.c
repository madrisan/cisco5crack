/* This file is part of `C5C'.
 * Copyright (C) 2002-2005 by Davide Madrisan <davide.madrisan@gmail.com>

 * This program is free software; you can redistribute it and/or modify it under
 * the terms of version 2 of the GNU General Public License as published by the
 * Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY, to the extent permitted by law; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.

 * This code is derived from the RSA Data Security, Inc. MD5 Message-Digest
 * Algorithm.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#if HAVE_INTTYPES_H
# include <inttypes.h>
#endif
#if HAVE_STRING_H
# if !STDC_HEADERS && HAVE_MEMORY_H
#  include <memory.h>
# endif
# include <string.h>
#endif
#if HAVE_STRINGS_H
# include <strings.h>
#endif
#if !HAVE_MEMSET
extern void *memset(void *s, int c, int n);
#endif

#include "md5.h"

static void md5_transform(u_int32_t state[4], u_int32_t block[16]);
static void bytes_encode(u_char *output, u_int32_t *input, u_int len);
static void bytes_decode(u_int32_t *output, u_char *input, u_int len);

/* These are the four functions used in the four steps of the MD5 algorithm
 * and defined in the RFC 1321.  The first function is a little bit optimized
 * (as found in Colin Plumbs public domain implementation).
 * In each bit position F1 acts as a conditional: if X then Y else Z
 * the function F1 could have been defined using + instead of | since
 * (x)&(y) and ~(x)&(z) will never have 1's in the same bit position
 * if the bits of x, y, and z are independent and unbiased,
 * the each bit of F(x,y,z) will be independent and unbiased
 */
#define F1(x, y, z)  ((z) ^ ((x) & ((y) ^ (z))))
/* #define F1(x, y, z)  (((x) & (y)) | ((~x) & (z))) */
#define F2(x, y, z)  F1((z), (x), (y))
/* H is the bit-wise 'xor' or 'parity' function of its inputs */
#define F3(x, y, z)  ((x) ^ (y) ^ (z))
#define F4(x, y, z)  ((y) ^ ((x) | (~z)))

/* This is the central step in the MD5 algorithm. */
#define MD5STEP(f, w, x, y, z, data, s) \
     ( w += f(x, y, z) + data, w = w << s | w >> (32 - s), w += x );

static void
bytes_encode(u_char *output, u_int32_t *input, u_int len)
{
     uint_fast16_t i, j;

     /* save number of bits (64-bit representation of b mod 2^64, where
      * b is the length of the message before the padding)
      *
      * xxxx.xxxx.xxxx.xxxx.xxxx.xxxx.xxxx.xxxx  (`input[i]': 32-bit)
      * <--j+3--> <--j+2--> <--j+1--> <--j+0-->
      */
#if ! defined(SHORT_PASSWORDS) || defined(WORDS_BIGENDIAN)
     for (i = 0, j = 0; j < len; i++, j += 4) {
          output[j] = (u_char)(input[i] & 0xff);
          output[j+1] = (u_char)((input[i] >> 8) & 0xff);
          output[j+2] = (u_char)((input[i] >> 16) & 0xff);
          output[j+3] = (u_char)((input[i] >> 24) & 0xff);
     }
#else
     /* works only on little-endian architectures
      */
     for (i = 0, j = 0; j < len; i++, j += 4)
          memcpy(output + j, input + i, sizeof(u_int32_t));
#endif
}

static void
bytes_decode(u_int32_t *output, u_char *input, u_int len)
{
     uint_fast16_t i, j;

#if ! defined(SHORT_PASSWORDS) || defined(WORDS_BIGENDIAN)
     for (i = 0, j = 0; j < len; i++, j += 4)
          output[i] = ((u_int32_t)input[j]) |
                      (((u_int32_t)input[j+1]) << 8) |
                      (((u_int32_t)input[j+2]) << 16) |
                      (((u_int32_t)input[j+3]) << 24);
#else
     /* works only on little-endian architectures
      */
     for (i = 0, j = 0; j < len; i++, j += 4)
          memcpy(output + i, input + j, sizeof(u_int32_t));
#endif
}

/* MD5 initialization; begins an MD5 operation, writing a new context
 * (RFC 1321, 3.3: Step 3) */
extern void
md5_init(md5_context * context)
{
     context->bits[0] = context->bits[1] = 0;

     /* load four 32-bit initial chaining constants (IVs)
      * the registers are initialized to the following values in hexadecimal,
      * low-order bytes first):
      *    word A: 01 23 45 67
      *    word B: 89 ab cd ef
      *    word C: fe dc ba 98
      *    word D: 76 54 32 10
      */
     context->hash[0] = 0x67452301;
     context->hash[1] = 0xefcdab89;
     context->hash[2] = 0x98badcfe;
     context->hash[3] = 0x10325476;
}

/* MD5 basic transformation; transforms state based on block
 * the 64 additive constants are calculated using the formula
 *   c[j] = first 32 bits of binary value abs(sin(j+1)), 0 <= j <= 63,
 * (or the integer part of 4294967296 times abs(sin(i)))
 * where j is in radians
 *
 * here the order for accessing source words (in) and the number of bit
 * positions for left shifts (rotates)
 * - round 1
 *     z[ 0..15] = [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15]
 *     s[ 0..15] = [7,12,17,22,7,12,17,22,7,12,17,22,7,12,17,22]
 * - round 2
 *     z[16..31] = [1,6,11,0,5,10,15,4,9,14,3,8,13,2,7,12]
 *     s[16..31] = [5,9,14,20,5,9,14,20,5,9,14,20,5,9,14,20]
 * - round 3
 *     z[32..47] = [5,8,11,14,1,4,7,10,13,0,3,6,9,12,15,2]
 *     s[32..47] = [4,11,16,23,4,11,16,23,4,11,16,23,4,11,16,23]
 * - round 4
 *     z[48..63] = [0,7,14,5,12,3,10,1,8,15,6,13,4,11,2,9]
 *     s[48..63] = [6,10,15,21,6,10,15,21,6,10,15,21,6,10,15,21]
 */
static void
md5_transform(u_int32_t state[4], u_int32_t block[16])
{
    register u_int32_t a, b, c, d;
    u_int32_t z[16];

    bytes_decode(z, (u_char *) block, 64);

    a = state[0];
    b = state[1];
    c = state[2];
    d = state[3];

/* *INDENT-OFF* */
    MD5STEP(F1, a, b, c, d, z[ 0] + 0xd76aa478,  7);
    MD5STEP(F1, d, a, b, c, z[ 1] + 0xe8c7b756, 12);
    MD5STEP(F1, c, d, a, b, z[ 2] + 0x242070db, 17);
    MD5STEP(F1, b, c, d, a, z[ 3] + 0xc1bdceee, 22);
    MD5STEP(F1, a, b, c, d, z[ 4] + 0xf57c0faf,  7);
    MD5STEP(F1, d, a, b, c, z[ 5] + 0x4787c62a, 12);
    MD5STEP(F1, c, d, a, b, z[ 6] + 0xa8304613, 17);
    MD5STEP(F1, b, c, d, a, z[ 7] + 0xfd469501, 22);
    MD5STEP(F1, a, b, c, d, z[ 8] + 0x698098d8,  7);
    MD5STEP(F1, d, a, b, c, z[ 9] + 0x8b44f7af, 12);
    MD5STEP(F1, c, d, a, b, z[10] + 0xffff5bb1, 17);
    MD5STEP(F1, b, c, d, a, z[11] + 0x895cd7be, 22);
    MD5STEP(F1, a, b, c, d, z[12] + 0x6b901122,  7);
    MD5STEP(F1, d, a, b, c, z[13] + 0xfd987193, 12);
    MD5STEP(F1, c, d, a, b, z[14] + 0xa679438e, 17);
    MD5STEP(F1, b, c, d, a, z[15] + 0x49b40821, 22);

    MD5STEP(F2, a, b, c, d, z[ 1] + 0xf61e2562,  5);
    MD5STEP(F2, d, a, b, c, z[ 6] + 0xc040b340,  9);
    MD5STEP(F2, c, d, a, b, z[11] + 0x265e5a51, 14);
    MD5STEP(F2, b, c, d, a, z[ 0] + 0xe9b6c7aa, 20);
    MD5STEP(F2, a, b, c, d, z[ 5] + 0xd62f105d,  5);
    MD5STEP(F2, d, a, b, c, z[10] + 0x02441453,  9);
    MD5STEP(F2, c, d, a, b, z[15] + 0xd8a1e681, 14);
    MD5STEP(F2, b, c, d, a, z[ 4] + 0xe7d3fbc8, 20);
    MD5STEP(F2, a, b, c, d, z[ 9] + 0x21e1cde6,  5);
    MD5STEP(F2, d, a, b, c, z[14] + 0xc33707d6,  9);
    MD5STEP(F2, c, d, a, b, z[ 3] + 0xf4d50d87, 14);
    MD5STEP(F2, b, c, d, a, z[ 8] + 0x455a14ed, 20);
    MD5STEP(F2, a, b, c, d, z[13] + 0xa9e3e905,  5);
    MD5STEP(F2, d, a, b, c, z[ 2] + 0xfcefa3f8,  9);
    MD5STEP(F2, c, d, a, b, z[ 7] + 0x676f02d9, 14);
    MD5STEP(F2, b, c, d, a, z[12] + 0x8d2a4c8a, 20);

    MD5STEP(F3, a, b, c, d, z[ 5] + 0xfffa3942,  4);
    MD5STEP(F3, d, a, b, c, z[ 8] + 0x8771f681, 11);
    MD5STEP(F3, c, d, a, b, z[11] + 0x6d9d6122, 16);
    MD5STEP(F3, b, c, d, a, z[14] + 0xfde5380c, 23);
    MD5STEP(F3, a, b, c, d, z[ 1] + 0xa4beea44,  4);
    MD5STEP(F3, d, a, b, c, z[ 4] + 0x4bdecfa9, 11);
    MD5STEP(F3, c, d, a, b, z[ 7] + 0xf6bb4b60, 16);
    MD5STEP(F3, b, c, d, a, z[10] + 0xbebfbc70, 23);
    MD5STEP(F3, a, b, c, d, z[13] + 0x289b7ec6,  4);
    MD5STEP(F3, d, a, b, c, z[ 0] + 0xeaa127fa, 11);
    MD5STEP(F3, c, d, a, b, z[ 3] + 0xd4ef3085, 16);
    MD5STEP(F3, b, c, d, a, z[ 6] + 0x04881d05, 23);
    MD5STEP(F3, a, b, c, d, z[ 9] + 0xd9d4d039,  4);
    MD5STEP(F3, d, a, b, c, z[12] + 0xe6db99e5, 11);
    MD5STEP(F3, c, d, a, b, z[15] + 0x1fa27cf8, 16);
    MD5STEP(F3, b, c, d, a, z[ 2] + 0xc4ac5665, 23);

    MD5STEP(F4, a, b, c, d, z[ 0] + 0xf4292244,  6);
    MD5STEP(F4, d, a, b, c, z[ 7] + 0x432aff97, 10);
    MD5STEP(F4, c, d, a, b, z[14] + 0xab9423a7, 15);
    MD5STEP(F4, b, c, d, a, z[ 5] + 0xfc93a039, 21);
    MD5STEP(F4, a, b, c, d, z[12] + 0x655b59c3,  6);
    MD5STEP(F4, d, a, b, c, z[ 3] + 0x8f0ccc92, 10);
    MD5STEP(F4, c, d, a, b, z[10] + 0xffeff47d, 15);
    MD5STEP(F4, b, c, d, a, z[ 1] + 0x85845dd1, 21);
    MD5STEP(F4, a, b, c, d, z[ 8] + 0x6fa87e4f,  6);
    MD5STEP(F4, d, a, b, c, z[15] + 0xfe2ce6e0, 10);
    MD5STEP(F4, c, d, a, b, z[ 6] + 0xa3014314, 15);
    MD5STEP(F4, b, c, d, a, z[13] + 0x4e0811a1, 21);
    MD5STEP(F4, a, b, c, d, z[ 4] + 0xf7537e82,  6);
    MD5STEP(F4, d, a, b, c, z[11] + 0xbd3af235, 10);
    MD5STEP(F4, c, d, a, b, z[ 2] + 0x2ad7d2bb, 15);
    MD5STEP(F4, b, c, d, a, z[ 9] + 0xeb86d391, 21);
/* *INDENT-ON* */

     /* '+=' promotes a faster 'avalanche effect' */
     state[0] += a;
     state[1] += b;
     state[2] += c;
     state[3] += d;

#ifndef SHORT_PASSWORDS
     /* zeroize sensitive information */
     memset((void *) z, 0, sizeof(z));
#endif

}

/* MD5 block update operation; continues an MD5 message-digest operation,
 * processing another message block, and updating the context
 */
extern void
md5_update(md5_context * context, const u_char * input, u_int inputlen)
{
     u_int32_t t;

     /* update bitcount */
     t = context->bits[0];
     if ((context->bits[0] = t + ((u_int32_t) inputlen << 3)) < t)
          context->bits[1]++;         /* carry from low to high */
#ifndef SHORT_PASSWORDS
     /* `inputlen' is never greater than 2^32 bits = 2^29 bytes,
      * in case of `SHORT_PASSWORDS'
      */
     context->bits[1] += inputlen >> 29;
#endif

     t = (t >> 3) & 0x3f;
     /* handle any leading odd-sized chunks */
     if (t) {
          u_char *p = (u_char *) context->block + t;
          t = 64 - t;
          if (inputlen < t) {
               memcpy(p, input, inputlen);
               return;
          }

          memcpy(p, input, t);
          md5_transform(context->hash, (u_int32_t *) context->block);
          input += t;
          inputlen -= t;
     }

#ifndef SHORT_PASSWORDS
     /* process data in 64-byte chunks */
     while (inputlen >= 64) {
          memcpy(context->block, input, 64);
          md5_transform(context->hash, (u_int32_t *) input);
          input += 64;
          inputlen -= 64;
     }
#endif

     /* handle any remaining bytes of data. */
     memcpy(context->block, input, inputlen);
}

/* MD5 finalization; ends an MD5 message-digest operation, writing the
 * the message digest and zeroizing the context
 */
extern void
md5_final(u_char digest[16], md5_context * context)
{
     uint_fast16_t count;
     u_char *p;

     /* compute number of bytes mod 64 */
     count = (context->bits[0] >> 3) & 0x3F;

     /* set the first char of padding to 0x80;
      * this is safe since there is always at least one byte free
      * note: '0x80' (base 16) == '1000.0000' (base 2)
      */
     p = context->block + count;
     *p++ = 0x80;

     /* bytes of padding needed to make 64 bytes
      * (RFC 1321, 3.1: Step 1)
      */
     count = 63 - count;

     /* pad out to 56 mod 64 */
     if (count < 8) {
          /* two lots of padding: pad the first block to 64 bytes */
          memset(p, 0, count);
          md5_transform(context->hash, (u_int32_t *) context->block);
          /* now fill the next block with 56 bytes */
          memset(context->block, 0, 56);
     } else
          memset(p, 0, count - 8);       /* pad block to 56 bytes */

     /* append length in bits and transform */
     bytes_encode((u_char*)((u_int32_t *) context->block + 14),
                  context->bits, 8);
     md5_transform(context->hash, (u_int32_t *) context->block);
     bytes_encode(digest, context->hash, 16);

#ifndef SHORT_PASSWORDS
     /* zeroize sensitive information */
     memset((char *) context, 0, sizeof(*context));
#endif
}
