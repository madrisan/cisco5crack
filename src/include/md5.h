/* This file is part of 'C5C'.
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

#ifndef _MD5_H
#define _MD5_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif

/* MD5 context */
typedef struct md5_context_t {
     u_int32_t hash[4];         /* state (ABCD) */
     u_char block[64];          /* input buffer */
     u_int32_t bits[2];         /* number of bits, modulo 2^64 (lsb first) */
} md5_context;

void md5_init(md5_context *);
void md5_update(md5_context *, const u_char *, u_int);
void md5_final(u_char[16], md5_context *);

#endif
