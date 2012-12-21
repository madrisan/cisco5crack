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

#if HAVE_STRING_H
# if !STDC_HEADERS && HAVE_MEMORY_H
#  include <memory.h>
# endif
# include <string.h>
#endif
#if HAVE_STRINGS_H
# include <strings.h>
#endif

#include "crypt.h"
#include "entropy.h"
#include "libcrypt.h"
#include "main.h"
#include "mem.h"
#include "params.h"
#include "tty.h"

extern u_char md5_itoa64[];
extern t_options usr_opt;

int
crypt_ios_fe(u_char * pw, u_char * salt)
{
     int error;

     if ((error = cisco_is_plaintext(pw)))
          tty_error(ERR_USAGE,
                    "invalid char `%c' (pos:%d) in plaintext to crypt",
                    (char) pw[error - 1], error);

     if (strlen((char *) pw) > MAX_PLAIN_SIZE)
          tty_error(ERR_USAGE,
                    "the plaintext length exceed `%d'", MAX_PLAIN_SIZE);

     /* no salt specified, so a random one is needed */
     if (!salt) {
          salt = zmem_alloc(MD5_SALT_SIZE + 1);
          error = cisco_forge_salt(
               (u_char *)salt, MD5_SALT_SIZE, usr_opt.cryptopt.get_entropy);
          if (error) {
               tty_warning("falling back to ANSI rand()");
               cisco_forge_salt((u_char *)salt, MD5_SALT_SIZE, get_sys_entropy);
          }
     }
     else if ((error = cisco_is_salt(salt)))
          tty_error(ERR_USAGE,
                    "illegal char (`%c') in salt `%s'\n"
                    "allowed characters:\n%s",
                    (char) salt[error - 1], salt, md5_itoa64);

     if (usr_opt.verbose)
          tty_message("plain string     : %s\n"
                      "salt used        : %s\n"
                      "encrypted string : %s\n",
                      pw, salt, cisco_ios_crypt((u_char *)pw, (u_char *)salt));
     else                       /* quiet mode */
          tty_message("%s\n", cisco_ios_crypt((u_char *)pw, (u_char *)salt));

     return SUCCESS;
}

int
crypt_pix_fe(u_char * pw)
{
     int error_ch;

     if ((error_ch = cisco_is_plaintext(pw)))
          tty_error(ERR_USAGE, "invalid char `%c' in plaintext to crypt",
                    (char) pw[error_ch - 1]);

     if (strlen((char *) pw) > MD5_PIX_HASH_SIZE)
          tty_error(ERR_USAGE, "PIX password can't exceed %d characters",
                    MD5_PIX_HASH_SIZE);

     if (usr_opt.verbose)
          tty_message("plain string     : %s\n"
                      "encrypted string : %s\n",
                      pw, cisco_pix_crypt((u_char *)pw));
     else                       /* quiet mode */
          tty_message("%s\n", cisco_pix_crypt((u_char *)pw));

     return SUCCESS;
}
