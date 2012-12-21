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

#ifndef _MAIN_H
#define _MAIN_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif

typedef enum t_action {
     none = 0,
     crypt_ios, crypt_pix,
     decrypt_bf, decrypt_wl,
     resume, resume_bf, resume_wl
} t_action;

typedef struct t_options {      /* arguments passed on command line */
     t_action action;           /* crypt, decrypt, or resume ? */
     int verbose;               /* verbose level (`0' for quiet mode) */
#if defined (HAVE_WORKING_FORK)
     int daemonize;             /* fork and run in daemon mode */
#endif
     /* crypt options
      */
     struct {
          u_char *salt;
          u_char *plain;
          int (*get_entropy) (u_char * buf, size_t nbytes);
#if defined (EGD_SUPPORT)
          char *egd_path;
#endif
     } cryptopt;

     /* decrypt options
      */
     struct {
          int fromlen, tolen;
          char *saved_date;
          char *regexpr;        /* regular expression */
          char *wordlist;       /* dictionary to be used */
          char *restore_file;   /* name of restore file */
          u_char *cipher;       /* cisco password */
     } decryptopt;
} t_options;

#endif
