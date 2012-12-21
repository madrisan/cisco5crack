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
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#if HAVE_FCNTL_H
# include <fcntl.h>
#endif
#if HAVE_INTTYPES_H
# include <inttypes.h>
#endif
#include <stdio.h>
#if STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# if HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif
#if HAVE_STRING_H
# if !STDC_HEADERS && HAVE_MEMORY_H
#  include <memory.h>
# endif
# include <string.h>
# if !HAVE_MEMSET
void *memset(void *s, int c, int n);
# endif
#endif
#if HAVE_STRINGS_H
# include <strings.h>
#endif
#if !HAVE_STRCHR
# define strchr index
#endif
#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif
#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#if HAVE_UNISTD_H
# include <unistd.h>
#endif

#include "libcrypt.h"
#include "md5.h"

static void cisco_md5_to64(char *s, u_long v, int n);
#if WORDS_BIGENDIAN
static inline void byte_be2le(u_char *buf, u_int words);
#endif

/* note:
 * - the `?' character can't be used (it's the help request key)
 * - the space characters in a password are discarded *ONLY* if they appear
 *   at the begin of the password
 *    (ex. "a  "  -->  $1$kmai$nQYmoqAyylC5c1d7JulEA/)
 * - IOS silently discards null passwords
 */
#define ALPHABET_LOWER      "abcdefghijklmnopqrstuvwxyz"
#define ALPHABET_UPPER      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define ALPHABET_DIGIT      "0123456789"

#define MD5_MAGIC (const u_char *) "$1$"

u_char cisco_alphabet[] =
    " !\"#$%&'()*+,-./"
    ALPHABET_DIGIT
    ":;<=>@"
    ALPHABET_UPPER
    "[\\]^_`"
    ALPHABET_LOWER
    "{|}~";

u_char md5_itoa64[] =
    "./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

static void
cisco_md5_to64(char *s, u_long v, int n)
{
     while (--n >= 0) {
          /* the lowest 6 bits at every step
           *   ex. |123456|123456|123456|123456|   (n=4)
           *       | s[3] | s[2] | s[1] | s[0] |
           */
          *s++ = md5_itoa64[v & 0x3f];
          v >>= 6;
     }
}

#if WORDS_BIGENDIAN
/* Definitions for byte order, according to significance of bytes, from low
 * addresses to high addresses.  The value is what you get by putting '4'
 * in the most significant byte, '3' in the second most significant byte,
 * '2' in the second least significant byte, and '1' in the least significant
 * byte, and then writing down one digit for each byte, starting with the byte
 * at the lowest address at the left, and proceeding to the byte with the
 * highest address at the right.
 *
 * #define LITTLE_ENDIAN  1234
 * #define BIG_ENDIAN     4321
 * #define PDP_ENDIAN     3412
 */
/* BIG_ENDIAN sequence of words converted into the LITTLE_ENDIAN format */
static inline void
byte_be2le(u_char *buf, u_int words)
{
     u_int32_t t;

     do {
          t = (u_int32_t) ((u_int) buf[3] << 8 | buf[2]) << 16 |
              ((u_int) buf[1] << 8 | buf[0]);
          *(u_int32_t *) buf = t;
          buf += 4;
     } while (--words);
}
#endif

int
cisco_is_plaintext(u_char * plaintext)
{
     uint_fast8_t i;

     /* starting space characters in `plaintext' are illegal for IOS */
     if (*plaintext == ' ')
          return 1;

     for (i = 0; i < strlen((char *) plaintext); i++) {
          if (!strchr((char *)cisco_alphabet, plaintext[i]))
               return (i + 1);   /* to skip the zero value */
     }
     return 0;
}

int
cisco_is_salt(u_char * salt)
{
     u_char *cp;

     for (cp = salt; cp < (salt + MD5_SALT_SIZE); cp++) {
          if (!strchr((char *)md5_itoa64, *cp))
              return cp - salt + 1;   /* to skip the zero value */
     }
     return 0;
}

/* Puts `nbytes' random bytes into the buffer `buf', using as random source
 * the entropy function `get_entropy' or the ANSI `rand' function if the
 * function pointer is set to NULL.
 * Returns `1' if `get_entropy' cannot generate `nbytes' random bytes,
 * `0' otherwise.
 */

int
cisco_forge_salt(u_char * buf, u_int nbytes,
     int (*get_entropy) (u_char *, size_t))
{
     uint_fast16_t i, entropy_amount = 0;

     if (get_entropy) {
          entropy_amount = (* get_entropy)(buf, nbytes);
          if (entropy_amount < nbytes)
               return 1;
     }
     else {
          srand((u_int) time(NULL));
          /* to use higher-order bits of pseudo-random number */
          for (i = 0; i < nbytes; i++)
               buf[i] = (u_char) ((1. * rand() * 255) / RAND_MAX);
     }

     for (i = 0; i < nbytes; i++) {
          /* to use the 6 higher-order bits of `buf[i]' */
          buf[i] = md5_itoa64[(buf[i] >> 2) & 0x3F];
     }

     return 0;
}

/* ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <phk@login.dknet.dk> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Poul-Henning Kamp
 * ----------------------------------------------------------------------------
 *
 * UNIX password
 * use MD5 for what it is best at...
 */
char *
cisco_ios_crypt(u_char * pw, u_char * salt)   /* freebsd_crypt */
{
#ifndef SHORT_PASSWORDS
     md5_context ctx, ctx1;
     u_char final[16 + 1];      /* +1 only to aid in looping */
#else
     static md5_context ctx, ctx1;
     static u_char final[16 + 1];
#endif
     static char passwd[MD5_HASH_SIZE + 1], *p;
     uint_fast16_t pw_len = strlen((char *) pw);
     int_fast16_t i;
     u_long l;

     /* then just as many characters of the MD5(pw, salt, pw) */
     md5_init(&ctx1);
     md5_update(&ctx1, pw, pw_len);
     md5_update(&ctx1, salt, MD5_SALT_SIZE);
     md5_update(&ctx1, pw, pw_len);
     md5_final(final, &ctx1);

     md5_init(&ctx);
     md5_update(&ctx, pw, pw_len);      /* the password first */
     md5_update(&ctx, MD5_MAGIC, MD5_MAGIC_SIZE);   /* then our magic string */
     md5_update(&ctx, salt, MD5_SALT_SIZE);     /* then the raw salt */
#ifndef SHORT_PASSWORDS
     for (i = pw_len; i > 0; i -= 16)
          md5_update(&ctx, final, i > 16 ? 16 : i);
#else
     /* we assume that `pw' has length < 16 (chars) */
     md5_update(&ctx, final, pw_len);
#endif

     /* don't leave anything around in vm they could use */
     memset(final, 0, sizeof final);

     for (i = pw_len; i; i >>= 1)       /* then something really weird... */
          md5_update(&ctx, ((i & 1) ? final : pw), 1);
     md5_final(final, &ctx);

     /* and now, just to make sure things don't run too fast
      * on a 60 Mhz Pentium this takes 34 msec, so you would
      * need 30 seconds to build a 1000 entry dictionary...
      */
     for (i = 0; i < 1000; i++) {
          md5_init(&ctx1);
          if (i & 1)
               md5_update(&ctx1, pw, pw_len);
          else
               md5_update(&ctx1, final, 16);

          if (i % 3)
               md5_update(&ctx1, salt, MD5_SALT_SIZE);

          if (i % 7)
               md5_update(&ctx1, pw, pw_len);

          if (i & 1)
               md5_update(&ctx1, final, 16);
          else
               md5_update(&ctx1, pw, pw_len);
          md5_final(final, &ctx1);
     }

     /* now make the output string */
     memcpy(passwd, MD5_MAGIC, MD5_MAGIC_SIZE);
     memcpy(passwd + MD5_MAGIC_SIZE, salt, MD5_SALT_SIZE);
     passwd[MD5_PREAMBLE_SIZE - 1] = '$';

     p = passwd + MD5_PREAMBLE_SIZE;

     /* note: remove/use next line and you'll get:
      *
      * $1$aesy$JX0TcVaS4wonMdEDfV.mT0    used
      * $1$aesy$JX0TcVaS4wonMdED.U.mT0    removed
      *
      * $1$YURI$6GtF8FYidcHSVJkNbS3PW/    used
      * $1$YURI$6GtF8FYidcHSVJkN.Q3PW/    removed
      *
      * 012345678901234567890123456789
      * 0         1         2
      *                         --        the only two bytes involved
      *                         .         always = `.' (0) if removed
      *
      * `final[16]' only modify `passwd[24]' and `passwd[25]'
      * proof:  the value final[16] is only used in final[i+12] with i=4;
      * when i=4, the 4 bytes modified are passwd[24..27], but final[16]
      * is only xored with the lowest 8 bits of `l', so only two bytes
      * (8=2+6) are affected by the value of final[16]: passwd[24] and
      * passwd[25].
      */
     final[16] = final[5];      /* because [5] is not used in the loop */

     /* set 20 chars (5 steps * (4 chars)) of `passwd', in the positions
      * [MAGIC_SIZE + SALT_SIZE + 1 .. CIPHERTEXT_SIZE - 2] = [8 .. 27]
      */
     for (i = 0; i < 5; i++) {
          /* i   i+6   i+12
           * --------------
           * 0     6     12   xor --> (3 * (uchar = 8 bit)) = 24 bit
           * 1     7     13
           * 2     8     14
           * 3     9     15
           * 4    10     16
           * .     .          (5 and 11 are not used in this loop)
           */
          l = (final[i] << 16) | (final[i + 6] << 8) | final[i + 12];
          /* converts `l' into the string `p' (4 chars), using `md5_itoa64'
           * note that every char in `p' uses only 6 bits of `l', so
           * 24 bits are sufficient to make a 4-byte string
           */
          cisco_md5_to64(p, l, 4);
          p += 4;               /* next 4 bytes */
     }

     /* now set the last two chars of `passwd' */
     l = final[11];             /* 11 because its value was not used in the loop */
     /* convert l into a 2-byte string
      * only 8 bits can be equal to 1 (`final[11]' is 1 uchar), so the last
      * char of passwd is selected from `md5_itoa64' using a two bit number
      * (final is 128-bit (16*8) long; we need 132 bits (`CIPHERTEXT_SIZE'*6))
      *
      * that's why the cisco passwords end with one char from the set '/.01'
      */
     cisco_md5_to64(p, l, 2);

     *(p + 2) = '\0';           /* null char at the end of `passwd' */

/* #ifndef SHORT_PASSWORDS  (disabled for security reasons) */
     /* don't leave anything around in vm they could use */
     memset(final, 0, sizeof final);
/* #endif */
     return passwd;
}

char *
cisco_pix_crypt(u_char * pw)
{
     md5_context ctx1;
     static u_char final[MD5_PIX_HASH_SIZE + 1];
     static u_char cleartext[MD5_PIX_HASH_SIZE + 1];
     static char encoded[MD5_PIX_HASH_SIZE + 1];

     char *p = encoded;
     uint_fast8_t i;
     u_long v;

     memset(encoded, 0, MD5_PIX_HASH_SIZE + 1);
     memset(cleartext, 0, MD5_PIX_HASH_SIZE + 1);
     strcpy((char*) cleartext, (char*) pw);

     md5_init(&ctx1);
     md5_update(&ctx1, cleartext, MD5_PIX_HASH_SIZE);
     md5_final(final, &ctx1);

#if WORDS_BIGENDIAN
     byte_be2le(final, 4);
#endif
/*   cisco_md5_to64(p, *(u_long *) final, 4);
 *   cisco_md5_to64(p + 4, *(u_long *) (final + 4), 4);
 *   cisco_md5_to64(p + 8, *(u_long *) (final + 8), 4);
 *   cisco_md5_to64(p + 12, *(u_long *) (final + 12), 4);
 */
     v = *(u_long *) final;
     for (i = 1; i <= 16; i++) {
          *p++ = md5_itoa64[v & 0x3f];
#if 0
          fprintf(stderr, "p[%2d] = %c\n", i, *(p - 1));
#endif
          v >>= 6;
          if (!(i % 4))
              v = *(u_long *) (final + i);
     }
/*   *(p + MD5_PIX_HASH_SIZE) = '\0';
 */

     return encoded;
}

int
cisco_is_ios_md5crypt(u_char * ciphertext)
{
     /* the format of the cisco-type5 strings is :
      *
      *   $1$....$......................$
      *       4            22             (characters)
      *
      * here some examples :
      *
      *   $1$XjLF$8smrGgVnA7733oGwCRlIL/
      *   $1$r/M8$CyA3IbiT0fsm3x6MfZ.Gd1
      *   $1$CA8q$hoRVKiWfxeA3ZMjNOMnKe.
      *   $1$.2g8$GX2nkPf7PYSZ3CyDbDMJB1
      *   $1$Quvz$FqCNisajiE2FrLWbWWgQg0
      */
     u_char *ep, *sp = ciphertext;

     if (strncmp((char *) sp, (const char *) MD5_MAGIC, MD5_MAGIC_SIZE))
          return 0;
     sp += MD5_MAGIC_SIZE;

     /* it stops at the first `$', max `SALT_SIZE' chars */
     for (ep = sp; *ep && *ep != '$' && ep < (sp + MD5_SALT_SIZE); ep++)
          /* check if `*ep' is an allowed char */
          if (!strchr((char *) md5_itoa64, *ep))
               return 0;
     /* check the length of the true salt */
     if (*ep != '$' || (ep - sp) != MD5_SALT_SIZE)
          return 0;
     sp += MD5_SALT_SIZE;

     /* it stops at the end of the ciphertext, max `CIPHERTEXT_SIZE' chars */
     for (ep = ++sp; *ep && ep < (sp + MD5_CIPHERTEXT_SIZE); ep++)
          /* check if `*ep' is an allowed char */
          if (!strchr((char *) md5_itoa64, *ep))
               return 0;
     /* check the length of the true ciphertext */
     if ((ep - sp) != MD5_CIPHERTEXT_SIZE)
          return 0;

     return 1;
}

int
cisco_is_pix_md5crypt(u_char * ciphertext)
{
     /* the cisco PIX cripted strings are 16 chars long, with each
      * character in the alphabet `md5_itoa64' (see `md5.h')
      *
      * here some examples :
      *
      *   2KFQnbNIdI.2KYOU  ("cisco")
      *   5Rg/X0SvG9kIhoUP
      */
     u_char *ep, *sp = ciphertext;

     /* it stops at the first illegal char */
     for (ep = sp; *ep; ep++)
          /* check if `*ep' is an allowed char */
          if (!strchr((char *) md5_itoa64, *ep))
               return 0;

     if (ep - sp != MD5_PIX_HASH_SIZE)
          return 0;

     return 1;
}
