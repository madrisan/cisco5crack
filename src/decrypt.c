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

#if HAVE_ERRNO_H
# include <errno.h>
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
#endif
#if HAVE_STRINGS_H
# include <strings.h>
#endif
#if !HAVE_MEMCPY
# define memcpy(d, s, n) bcopy ((s), (d), (n))
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
#if HAVE_UNISTD_H
# include <unistd.h>     /* read */
#endif

#include "decrypt.h"
#include "libcrypt.h"
#include "main.h"
#include "mem.h"
#include "params.h"
#include "regex.h"
#include "tty.h"

static u_char bruteforce(void);
static u_char wordlist(void);

#if defined(HAVE_TERMIOS_H) && defined(HAVE_DEV_TTY)
extern int tty_fd;
#endif
extern char seekpoint[FLINE_BUFSIZE];
extern t_options usr_opt;

int *chpos, *chpos_safe, actual_len;
/* buffer for words taken from `wfile' (see `wordlist()') */
char wbuf[FLINE_BUFSIZE];

enum e_cipher_engine { cisco_ios_cipher, cisco_pix_cipher } cipher_engine;
typedef enum e_bool { false = 0, true } bool;

#define ALPHABET_FULL   "[:all:]"

/* difftime redefinition to avoid an unnecessary floating point dependency
 */
#define difftime(t1,t0) (double)(t1 - t0)

static u_char
bruteforce(void)
{
     u_char salt[MD5_SALT_SIZE], *plaintext;
#if defined(HAVE_TERMIOS_H) && defined(HAVE_DEV_TTY)
     char ch;
#endif
     char **alphabet;
     int *chposi, *vsize, i, j;
#if defined (HAVE_WORKING_FORK)
     FILE *fout;
#endif

     /* initialize data */
     strncpy((char *) salt,
             (char *) (usr_opt.decryptopt.cipher + MD5_MAGIC_SIZE),
             MD5_SALT_SIZE);

     plaintext = zmem_alloc(MAX_PLAIN_SIZE + 1);
     if (usr_opt.action != resume_bf)
          chpos = zmem_alloc(MAX_PLAIN_SIZE * sizeof(*chpos));
     chpos_safe = zmem_alloc(MAX_PLAIN_SIZE * sizeof(*chpos_safe));
     vsize = zmem_alloc(MAX_PLAIN_SIZE * sizeof(*vsize));

     /* set the alphabet(s) used reading/parsing the relate argv */
     alphabet = zmem_alloc(MAX_PLAIN_SIZE * sizeof(*alphabet));

     /* option `-b' with no `regexpr' set */
     if (!usr_opt.decryptopt.regexpr)
          usr_opt.decryptopt.regexpr = str_alloc_copy(ALPHABET_FULL);

     regex_explode(alphabet, usr_opt.decryptopt.regexpr,
                   &usr_opt.decryptopt.fromlen, &usr_opt.decryptopt.tolen);

     if (usr_opt.action == resume_bf) {
          for (i = 0; i < usr_opt.decryptopt.tolen; i++) {
               vsize[i] = strlen(alphabet[i]);
               if (chpos[i] > vsize[i])
                    tty_error(ERR_INFO_RFILE,
                              "illegal `chpos' value load from restore file");
          }

          /* no errors in the restore file, so it can be safely removed here */
          if (remove(usr_opt.decryptopt.restore_file) == -1)
               tty_warning("cannot remove the file `%s'",
                            usr_opt.decryptopt.restore_file);
     } else {
          actual_len = usr_opt.decryptopt.fromlen;
          for (i = 0; i < usr_opt.decryptopt.tolen; i++) {
               chpos[i] = chpos_safe[i] = 0;
               vsize[i] = strlen(alphabet[i]);
          }
     }

     if (usr_opt.verbose) {
          /* some infos, just to let the user check the input entered */
          tty_message("trying to crack   : \"%s\"\n"
                      "method used       : bruteforce\n"
                      "password scheme   : ",
                      usr_opt.decryptopt.cipher);

          tty_message("%s %s\n", usr_opt.decryptopt.regexpr,
                      !strcmp(usr_opt.decryptopt.regexpr, ALPHABET_FULL) ?
                      "(full search)" : "");
          if (usr_opt.verbose > 2) {
               tty_message("expanded password scheme :\n");
               i = 0;
               while (i < usr_opt.decryptopt.tolen) {
                    for (j = i + 1;
                         j < usr_opt.decryptopt.tolen &&
                         alphabet[j] == alphabet[i]; j++);
                    if (j - 1 > i) {
                         tty_message("   p[%d..%d] = \"%s\"\n",
                                     i, j - 1, alphabet[i]);
                         i = j;
                    } else {
                         tty_message("   p[%d] = \"%s\"\n", i, alphabet[i]);
                         i++;
                    }
               }
          }

          tty_message("length of strings : ");
          if (usr_opt.decryptopt.fromlen !=
              usr_opt.decryptopt.tolen)
               tty_message("[%d..%d]\n",
                           usr_opt.decryptopt.fromlen,
                           usr_opt.decryptopt.tolen);
          else
               tty_message("%d\n", usr_opt.decryptopt.fromlen);

          if (usr_opt.action == resume_bf)
               tty_message("\nrestoring session stopped on %s\n",
                           usr_opt.decryptopt.saved_date);

          tty_message("\nplease wait... ");

#if defined(HAVE_TERMIOS_H) && defined(HAVE_DEV_TTY)
          tty_message("\n[type <ENTER> to display the current plaintext] ");
#endif
     }

     /* initialize the first plaintext to be used */
     for (i = 0; i < actual_len; i++)
          plaintext[i] = alphabet[i][chpos[i]];

     while (1) {
#if defined(HAVE_TERMIOS_H) && defined(HAVE_DEV_TTY)
          if (read(tty_fd, &ch, 1) > 0 && usr_opt.verbose)
               tty_message("%s", plaintext);
#endif
#if 0
          /* begin of DEBUG CODE */
          fprintf(stderr, "\n%s\n", plaintext);
          for (i = 0; i < usr_opt.decryptopt.tolen; i++)
               fprintf(stderr, "chpos[%d] = %d\n", i, chpos[i]);
          fflush(stderr);
          /* end of DEBUG CODE */
#endif

          if (!strcmp((cipher_engine == cisco_ios_cipher ?
                      cisco_ios_crypt(plaintext, salt) :
                      cisco_pix_crypt(plaintext)) + MD5_PREAMBLE_SIZE,
                      (char *) (usr_opt.decryptopt.cipher +
                                MD5_PREAMBLE_SIZE))) {
#if defined(HAVE_WORKING_FORK)
               if (usr_opt.daemonize) {
                    if ((fout = fopen(SCORE_FILE, "a"))) {
                         fprintf(fout, "** FOUND: \"%s\" (%s)\n",
                                 plaintext, usr_opt.decryptopt.cipher);
                         fclose(fout);
                    }
               }
               else
#endif
                    tty_message(usr_opt.verbose ?
                                "\n** FOUND: \"%s\"\n" : "%s\n", plaintext);
               i = 0;
               while (i < actual_len) {
                    mem_free(alphabet[i++]);
                    /* skip the duplicates alphabets */
                    while (alphabet[i] == alphabet[i - 1])
                         i++;
               }
               return SUCCESS;
          }

          /* if `++chpos[i] % vsize[i]' is zero then all the letters of
           * `alphabet[i]' has been checked, so skip to the previous
           * letter (`i--') of `plaintext'
           */
          i = 0;
          memcpy(chpos_safe, chpos, MAX_PLAIN_SIZE * sizeof(*chpos_safe));
          do {
               chposi = chpos + i;
               *chposi = ++(*chposi) % vsize[i];
               plaintext[i] = alphabet[i][*chposi];
          } while (++i != actual_len && !*chposi);

          if (i == actual_len && !chpos[actual_len - 1]) {
               if (actual_len++ == usr_opt.decryptopt.tolen) {
#if defined (HAVE_WORKING_FORK)
                    if (usr_opt.daemonize) {
                         if ((fout = fopen(SCORE_FILE, "a"))) {
                              fprintf(fout,
                                      "** FAILURE: (%s)\n"
                                      "   password scheme   : %s\n"
                                      "   length of strings : [%d..%d]\n",
                                      usr_opt.decryptopt.cipher,
                                      usr_opt.decryptopt.regexpr,
                                      usr_opt.decryptopt.fromlen,
                                      usr_opt.decryptopt.tolen);
                              fclose(fout);
                         }
                    }
                    else
#endif
                         tty_message("\n** FAILURE: the cipher string doesn't "
                                     "match the password scheme you set\n");
                    i = 0;
                    while (i < actual_len) {
                         mem_free(alphabet[i++]);
                         /* skip the duplicates alphabets */
                         while (alphabet[i] == alphabet[i - 1])
                              i++;
                    }
                    return FAILURE;
               }
               tty_message("\n** NOTE: switching to lenght `%ld', "
                           "please wait... ", actual_len);
               plaintext[actual_len - 1] = alphabet[actual_len - 1][0];
          }
     }
}

static u_char
wordlist(void)
{
     FILE *wfile;
#if defined (HAVE_WORKING_FORK)
     FILE *fout;
#endif
     u_char salt[MD5_SALT_SIZE], len;
#if defined(HAVE_TERMIOS_H) && defined(HAVE_DEV_TTY)
     char ch;
#endif
     bool seekpoint_found = false;

     if ((wfile = fopen(usr_opt.decryptopt.wordlist, "rt")) == NULL)
          tty_error(ERR_IO_FILE,
                    "error opening `%s' : %s",
                    usr_opt.decryptopt.wordlist, strerror(errno));

     strncpy((char *) salt,
             (char *) (usr_opt.decryptopt.cipher + MD5_MAGIC_SIZE),
             MD5_SALT_SIZE);

     if (usr_opt.action != resume_wl) {
          if (!usr_opt.decryptopt.fromlen)   /* not set by the user */
               usr_opt.decryptopt.fromlen++;
          if (!usr_opt.decryptopt.tolen)     /* not set by the user */
               usr_opt.decryptopt.tolen = MAX_PLAIN_SIZE;
     }

     if (usr_opt.verbose) {
          /* some infos, just to let the user check the input entered */
          tty_message("trying to crack   : \"%s\"\n"
                      "method used       : dictionary\n"
                      "wordlist          : %s\n",
                      usr_opt.decryptopt.cipher,
                      usr_opt.decryptopt.wordlist);

          tty_message("length of strings : ");
          if (usr_opt.decryptopt.fromlen != usr_opt.decryptopt.tolen)
               tty_message("[%d..%d]\n",
                           usr_opt.decryptopt.fromlen,
                           usr_opt.decryptopt.tolen);
          else
               tty_message("%d\n", usr_opt.decryptopt.fromlen);

          if (usr_opt.action == resume_wl)
               tty_message("\nrestoring session stopped on %s\n",
                           usr_opt.decryptopt.saved_date);

          tty_message("\nplease wait ... ");
#if defined(HAVE_TERMIOS_H) && defined(HAVE_DEV_TTY)
          tty_message("\n[type <ENTER> to dispay the current plaintext used] ");
#endif
     }

     while (fgets(wbuf, FLINE_BUFSIZE, wfile) != NULL) {
          /* strip the trail character `\n' */
          if (wbuf[(len = strlen(wbuf)) - 1] == '\n')
               wbuf[--len] = '\0';

          /* switch to `seekpoint', if the action is `restore_wf' */
          if (!seekpoint_found && (usr_opt.action == resume_wl)
               && strcmp(seekpoint, wbuf))
               continue;
          else
               seekpoint_found = true;

          if (len < usr_opt.decryptopt.fromlen ||
              len > usr_opt.decryptopt.tolen)
               continue;        /* wrong range for length */

#if defined(HAVE_TERMIOS_H) && defined(HAVE_DEV_TTY)
          if (read(tty_fd, &ch, 1) > 0 && usr_opt.verbose)
               tty_message("%s", wbuf);
#endif
          if (!strcmp((cipher_engine == cisco_ios_cipher ?
                      cisco_ios_crypt((u_char *) wbuf, salt) :
                      cisco_pix_crypt((u_char *) wbuf)) + MD5_PREAMBLE_SIZE,
                      (char *) (usr_opt.decryptopt.cipher +
                                MD5_PREAMBLE_SIZE))) {

#if defined (HAVE_WORKING_FORK)
               if (usr_opt.daemonize) {
                    if ((fout = fopen(SCORE_FILE, "a"))) {
                         fprintf(fout, "** FOUND: \"%s\" (%s)\n",
                                 wbuf, usr_opt.decryptopt.cipher);
                         fclose(fout);
                    }
               }
               else
#endif
                    tty_message(usr_opt.verbose ?
                                "\n\n** FOUND: \"%s\"\n" : "%s\n", wbuf);

               return SUCCESS;
          }
     }
     fclose(wfile);

#if defined (HAVE_WORKING_FORK)
     if (usr_opt.daemonize) {
          if ((fout = fopen(SCORE_FILE, "a"))) {
               fprintf(fout,
                       "** FAILURE: (%s)\n"
                       "   dictionary used   : %s\n"
                       "   length of strings : [%d..%d]\n",
                       usr_opt.decryptopt.cipher,
                       usr_opt.decryptopt.wordlist,
                       usr_opt.decryptopt.fromlen,
                       usr_opt.decryptopt.tolen);
               fclose(fout);
          }
     }
     else
#endif
          tty_message("\n\n"
                      "** FAILURE: no match found in the dictionary selected\n"
                      "please, use another dictionary or choose the "
                      "bruteforce attack.\n");
     return FAILURE;
}

u_char
cisco_decrypt(void)
{
     u_char retval;
     time_t start, stop;

     tty_init();
     time(&start);

     /* check if the arg is a valid md5 cisco IOS password */
     if (cisco_is_ios_md5crypt(usr_opt.decryptopt.cipher))
          cipher_engine = cisco_ios_cipher;
     /* check if the arg is a valid md5 PIX password */
     else if (cisco_is_pix_md5crypt(usr_opt.decryptopt.cipher))
          cipher_engine = cisco_pix_cipher;
     else
          tty_error(ERR_USAGE,
                    "neither a Cisco IOS nor a PIX password -- `%s'",
                    usr_opt.decryptopt.cipher);

     retval = (usr_opt.action == decrypt_wl || usr_opt.action == resume_wl) ?
              wordlist() : bruteforce();
     time(&stop);

     if (usr_opt.verbose)
          tty_message("scan time: %g second(s)\n\n", difftime(stop, start));

     return retval;
}
