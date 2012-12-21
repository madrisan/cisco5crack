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

#ifndef _LIBCRYPT_H
#define _LIBCRYPT_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif

#define MD5_MAGIC_SIZE        3
#define MD5_SALT_SIZE         4
#define MD5_CIPHERTEXT_SIZE  22
#define MD5_PREAMBLE_SIZE    MD5_MAGIC_SIZE+MD5_SALT_SIZE+1
#define MD5_HASH_SIZE        MD5_MAGIC_SIZE+MD5_SALT_SIZE+MD5_CIPHERTEXT_SIZE+1

#define MD5_PIX_HASH_SIZE    16

int cisco_is_plaintext(u_char * plaintext);
int cisco_is_salt(u_char * salt);
int cisco_is_ios_md5crypt(u_char * ciphertext);
int cisco_is_pix_md5crypt(u_char * ciphertext);

int cisco_forge_salt(
     u_char * buf, u_int nbytes, int (*get_entropy) (u_char *, size_t));

/* freeBSD-style MD5-based password hash implementation */
char *cisco_ios_crypt(u_char * pw, u_char * salt);
/* implementation used by the PIX firewall */
char *cisco_pix_crypt(u_char * pw);

#endif
