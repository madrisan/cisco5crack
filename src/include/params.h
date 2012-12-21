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

#ifndef _PARAMS_H
#define _PARAMS_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef DEV_RANDOM_SUPPORT
# define RND_DEV            "/dev/random"
#endif

#ifdef EGD_SUPPORT
# define EGD_SOCKET_PATH    "/tmp/entropy"
#endif

#define NULL_DEV            "/dev/null"

/* some global parameters
 */
#ifdef SHORT_PASSWORDS
# define MAX_PLAIN_SIZE      15 /* **MUST** be less than 16 */
#else
# define MAX_PLAIN_SIZE      48
#endif
#define MD5_SIZE_OVER_SPEED  0  /* see comments in the file `md5.c' */

#define FLINE_BUFSIZE        128

#define RESTORE_FILE        "/var/log/"PACKAGE"/restore_cc"
#define SCORE_FILE          "/var/log/"PACKAGE"/score_cc"

#define HEAD_RESTORE_TEXT   "; restore file for "PACKAGE
#define HEAD_SCORE_TEXT     "; score file for "PACKAGE

#endif
