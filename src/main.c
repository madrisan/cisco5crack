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

#include <assert.h>
#if HAVE_ERRNO_H
# include <errno.h>
#endif
#if HAVE_FCNTL_H
# include <fcntl.h>
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
#include <signal.h>
#if defined HAVE_UMASK
# if HAVE_SYS_STAT_H
#  include <sys/stat.h>
# endif
# if HAVE_SYS_TYPES_H
#  include <sys/types.h>
# endif
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
# include <unistd.h>
#endif

#include "getopt.h"

#include "crypt.h"
#include "decrypt.h"
#include "entropy.h"
#include "libcrypt.h"
#include "main.h"
#include "mem.h"
#include "params.h"
#include "session.h"
#include "tty.h"

#define PACKAGE_COPYLEFT  "Copyright (C) 2002, 2003 Davide Madrisan"
#define PACKAGE_SUBTITLE  "cracker for Cisco IOS and PIX MD5 passwords"

static void license(void);
static void version(void);
static void usage(int exit_code);
static void opt_parse(int argc, char **argv, t_options *opt);
static void opt_check(int *opt_set);

static const char *sys_def[] = {
#if defined (__alpha)
     "__alpha",
#endif
#if defined (__osf__)
     "__osf__",
#endif
#if defined (aix)
     "AIX",
#endif
#if defined (bsdi)
     "bsdi",
#endif
#if defined (__CYGWIN__)
     "Cygwin",
#endif
#if defined (__FreeBSD__)
     "FreeBSD",
#endif
#if defined (__linux__)
     "Linux",
#endif
#if defined (M_UNIX)
     "M_UNIX",
#endif
#if defined (__NetBSD__)
     "NetBSD",
#endif
#if defined (__OpenBSD__)
     "OpenBSD",
#endif
#if defined (sun)
     "SUN",
#endif
#if defined (sys5)
     "sys5",
#endif
#if defined (unixware)
     "unixware",
#error SCO operating systems unsupported.  Unixware sucks.
#endif
     NULL     /* sentinel */
};

static const char *compile_flags[] = {
#if defined (SHORT_PASSWORDS)
     "SHORT_PASSWORDS",
#endif
#if defined (DEV_RANDOM_SUPPORT)
     "DEV_RANDOM_SUPPORT",
#endif
#if defined (EGD_SUPPORT)
     "EGD_SUPPORT",
#endif
     NULL
};

/* *INDENT-OFF* */
static const char *license_text[] = {
PACKAGE " (" PACKAGE_SUBTITLE ") version " PACKAGE_VERSION,
PACKAGE_COPYLEFT,
"",
"This program is free software; you can redistribute it and/or modify it under",
"the termsof version 2 of the GNU General Public License as published by the",
"Free Software Foundation.",
"",
"This program is distributed in the hope that it will be useful, but WITHOUT",
"ANY WARRANTY, to the extent permitted by law;  without even the implied",
"warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.",
"See the GNU General Public License for more details.",
"",
"You should have received a copy of the GNU General Public License along with",
"this program; if not, write to the Free Software Foundation, Inc.,",
"59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.",
"",
(char *)0
};

static const char *banner_text[] = {
PACKAGE " (" PACKAGE_SUBTITLE ") version " PACKAGE_VERSION,
PACKAGE_COPYLEFT " <" PACKAGE_BUGREPORT ">.",
"",
"This is free software; see the source for copying conditions.  There is NO",
"warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.",
"",
(char *)0
};

/* options args for 'getopt_long()' */
static const char *optstring = "+cs:"
#if defined (EGD_SUPPORT)
"e::"
#endif
#if defined (DEV_RANDOM_SUPPORT)
"d"
#endif
#if defined (EGD_SUPPORT) || defined (DEV_RANDOM_SUPPORT)
"y"
#endif
"pb::w:l:"
#if defined (HAVE_WORKING_FORK)
"D"
#endif
"r::LhqvV";

static struct option longopts[] = {
     { "crypt-ios",  no_argument,       0, 'c' },
     { "salt",       required_argument, 0, 's' },
#if defined (EGD_SUPPORT)
     { "egd",        optional_argument, 0, 'e' },
#endif
#if defined (DEV_RANDOM_SUPPORT)
     { "dev-random", no_argument,       0, 'd' },
#endif
#if defined (EGD_SUPPORT) || defined (DEV_RANDOM_SUPPORT)
     { "sys-rand",   no_argument,       0, 'y' },
#endif
     { "crypt-pix",  no_argument,       0, 'p' },
     { "bruteforce", optional_argument, 0, 'b' },
     { "dictionary", required_argument, 0, 'w' },
     { "length",     required_argument, 0, 'l' },
#if defined (HAVE_WORKING_FORK)
     { "daemonize",  no_argument,       0, 'D' },
#endif
     { "restore",    optional_argument, 0, 'r' },
     { "license",    no_argument,       0, 'L' },
     { "help",       no_argument,       0, 'h' },
     { "quiet",      no_argument,       0, 'q' },
     { "verbose",    no_argument,       0, 'v' },
     { "version",    no_argument,       0, 'V' },
     { 0, 0, 0, 0 }
};

static const char *usage_text[] = {
"usage: " PACKAGE " [-q|-v] -c ["
#if defined (EGD_SUPPORT)
"-e[/path/to/socket]|"
#endif
#if defined (DEV_RANDOM_SUPPORT)
"-d|"
#endif
#if defined (EGD_SUPPORT) || defined (DEV_RANDOM_SUPPORT)
"-y|"
#endif
"-s<salt>] <string>",
"       " PACKAGE " [-q|-v] -p <crypted>",
#if defined (HAVE_WORKING_FORK)
"       " PACKAGE " [-q|-v] [-D] [-l<n>[:<m>]] -b[<regex>] <crypted>",
"       " PACKAGE " [-q|-v] [-D] [-l<n>[:<m>]] -w<dictionary> <crypted>",
"       " PACKAGE " [-q|-v] [-D] -r[<file>]",
#else
"       " PACKAGE " [-q|-v] [-l<n>[:<m>]] -b[<regex>] <crypted>",
"       " PACKAGE " [-q|-v] [-l<n>[:<m>]] -w<dictionary> <crypted>",
"       " PACKAGE " [-q|-v] -r[<file>]",
#endif
"       " PACKAGE " -h",
"       " PACKAGE " -L",
"       " PACKAGE " -V",
"",
"where the above options mean:",
"  -c, --crypt-ios    Crypt the plaintext <string> as Cisco IOS do.",
"  -s, --salt         Use the salt <salt> when crypting <string>.",
#ifdef EGD_SUPPORT
"  -e, --egd          Use the EGD socket to randomly forge the salt.",
"                     Default path to EGD socket is `" EGD_SOCKET_PATH "'.",
#endif
#ifdef DEV_RANDOM_SUPPORT
"  -d, --dev-random   Use the `" RND_DEV "' device to randomly forge the salt.",
#endif
#if defined (EGD_SUPPORT) || defined (DEV_RANDOM_SUPPORT)
"  -y, --sys-rand     Use the ANSI `rand()' function to forge the salt.",
#endif
"  -p, --crypt-pix    Crypt the plaintext <string> as Cisco PIX firewalls do",
"  -b, --bruteforce   Try to decrypt <crypted> using a bruteforce attack.",
"                     The optional regular expression <regex> is a list",
"                     of one or more of the above tokens:",
"                       - a single char, like `a',",
"                       - a list of characters, like `[abc]', `[a-z0-9]'",
"                         or`[@L\\.sTa-d\\\\]{6}' (6 times `@L.sTabcd\\'),",
"                       - a predefined class of characters, like `[:alnum:]'",
"                         or `[:digit:]{2}' (2 digits).",
"  -w, --dictionary   Try to decrypt <crypted> using the dictionary",
"                     specified by the text file <dictionary>.",
"  -l, --length       Only use strings with length <n> [to <m>].",
#if defined (HAVE_WORKING_FORK)
"  -D, --daemonize    Run in daemon mode. ",
"                     If the process is killed (by the `kill <pid>' command),",
"                     the session is saved in the file `"RESTORE_FILE"'",
"                     overwise the result of the cracking session is appended",
"                     in the text file `"SCORE_FILE"'.",
#endif
"  -r, --restore      Continues an interrupted cracking session, reading",
"                     point information from the specified file, or from",
"                     `"RESTORE_FILE"', if no file is specified.",
"  -h, --help         Display the short help you're reading and exit.",
"  -q, --quiet        Run in quiet mode.",
"  -v, --verbose      Run in verbose mode. Can be repeated more times.",
"  -L, --license      Display program license and exit.",
"  -V, --version      Display program version and exit.",
"",
"some examples:",
"   " PACKAGE " --crypt-ios --salt=xyzt c1sc0s3cr3t",
#ifdef DEV_RANDOM_SUPPORT
"   " PACKAGE " --crypt-ios --dev-random r@nd0m1z3",
#endif
#ifdef EGD_SUPPORT
"   " PACKAGE " --crypt-ios --egd=/tmp/entropy n0t@l1nuxb0x",
#endif
"   " PACKAGE " --crypt-pix weakpw",
"   " PACKAGE " --bruteforce --length=6 '$1$wait$BwsQnj6YxpKO9gdy4pkaA/'",
"   " PACKAGE " -v --bruteforce=[:lower:]{5} '$1$dDoS$FxuqrTObFaJCC8sIOq2XM.'",
"   " PACKAGE " -b[achrs] -l4:6 '$1$EaSy$ZwDb9InwxJQ02o93J4br61'",
"   " PACKAGE " -vv -b'[[:digit:][:lower:]]x[abc\\\\\\_][0-9]{2}f' \\",
"               '$1$dead$ZwzWtMyPbRpVyXJVvsnly/'",
"   " PACKAGE " -vv -b'[^[:upper:][!-(]]' '$1$WAiT$7jHUf3LiU/.ZDOY2NcIzE0'",
"   " PACKAGE " -vv --bruteforce=[:alpha:] /HyU4nBDhpLVZFD2",
"   " PACKAGE " --dictionary=./words.lst -l10 '$1$salt$q1U/vVn2cZb7JG3sxRad60'",
"",
"Please, use this software in a legal way!",
"For bugs and suggestions, please contact me by e-mail.",
"",
(char *)0
};
/* *INDENT-ON* */

static void
license(void)
{
     u_int i;

     for (i = 0; license_text[i]; i++)
          fprintf(stdout, "%s\n", license_text[i]);
}

static void
usage(int exit_code)
{
     u_int i;

     version();
     for (i = 0; usage_text[i]; i++)
          fprintf(stdout, "%s\n", usage_text[i]);

     exit(exit_code);
}

static void
version(void)
{
     u_int i;

     for (i = 0; banner_text[i]; i++)
          fprintf(stdout, "%s\n", banner_text[i]);

     fprintf(stdout, "Compilation platform: ");
     if (sys_def[0]) {
          for (i = 0; sys_def[i]; i++)
               fprintf(stdout, "%s ", sys_def[i]);
     } else
          fprintf(stdout, "unknown");

     fprintf(stdout, "\nCompilation flags: ");
     if (compile_flags[0]) {
          for (i = 0; compile_flags[i]; i++)
               fprintf(stdout, "%s ", compile_flags[i]);
     }
     fprintf(stdout, "\n\n");
}

/* *INDENT-OFF* */
enum e_error_matrix {   /* same order as in `opt_table' and `longopts' */
     opt_unset = 0,
     opt_c = opt_unset, opt_s, opt_e, opt_d, opt_y, opt_p,
     opt_b, opt_w, opt_l, opt_D, opt_r, opt_L, opt_h, opt_q, opt_v, opt_V,
     opt_last = opt_V
};
/* *INDENT-ON* */

static void opt_check(int * opt_set)
{
/* *INDENT-OFF* */
/* error table (0 means `ok', 1 means `illegal option pair')
 */
     static const char *opt_table[] = {
          /*       csedypbwlDrLhqvV                */
          /* c */ "",                /* crypt-ios  */
          /* s */ "0",               /* salt       */
          /* e */ "01",              /* egd        */
          /* d */ "011",             /* dev-random */
          /* y */ "0111",            /* sys-rand   */
          /* p */ "11111",           /* crypt-pix  */
          /* b */ "111111",          /* bruteforce */
          /* w */ "1111111",         /* dictionary */
          /* l */ "11111100",        /* length     */
          /* D */ "111111000",       /* daemonize  */
          /* r */ "1111111110",      /* restore    */
          /* L */ "11111111111",     /* license    */
          /* h */ "111111111111",    /* help       */
          /* q */ "0000000001011",   /* quiet      */
          /* v */ "00000000010111",  /* verbose    */
          /* V */ "111111111111111"  /* version    */
     };
/* *INDENT-ON* */

     int row, col;

     for (row = 0; row < opt_last; row++) {
          /* the matrix `opt_table' is symmetric */
          for (col = 0; col < row; col++) {
               if (*(opt_table[row] + col) == '1'
                   && opt_set[row] && opt_set[col])
                    tty_error(ERR_USAGE,
                              "`%s' and `%s' belong to different contexts",
                              longopts[row].name, longopts[col].name);
          }
     }
}

static void
opt_parse(int argc, char **argv, t_options *opt)
{
     int argval, n, opt_set[opt_last];

     /* initialize `opt_set' with the value 0 (option not used) */
     for (n = 0; n < opt_last; n++)
          opt_set[n] = opt_unset;

     /* process the options */
     opterr = 0;
     while (1) {
          argval = getopt_long(argc, argv, optstring, longopts, NULL);
          if (argval == -1)
               break;

          switch (argval) {
          case 0:
               break;
          case 'c':
               opt_set[opt_c]++;
               opt->action = crypt_ios;
               break;
          case 's':            /* set the salt string (4 character long) */
               opt_set[opt_s]++;
               opt->action = crypt_ios;
               if (strlen(optarg) != MD5_SALT_SIZE)
                    tty_error(ERR_USAGE,
                              "bad length (not %d bytes) for salt `%s'",
                              MD5_SALT_SIZE, optarg);
               opt->cryptopt.salt = (u_char *) str_alloc_copy(optarg);
               break;
#ifdef EGD_SUPPORT
          case 'e':
               opt_set[opt_e]++;
               opt->cryptopt.get_entropy = get_egd_entropy;
               if (optarg)
                    opt->cryptopt.egd_path = str_alloc_copy(optarg);
               break;
#endif
#ifdef DEV_RANDOM_SUPPORT
          case 'd':
               opt_set[opt_d]++;
               opt->cryptopt.get_entropy = get_dev_entropy;
               break;
#endif
#if defined (EGD_SUPPORT) || defined (DEV_RANDOM_SUPPORT)
          case 'y':
               opt_set[opt_y]++;
               opt->cryptopt.get_entropy = get_sys_entropy;
               break;
#endif
          case 'p':
               opt_set[opt_p]++;
               opt->action = crypt_pix;
               break;
          case 'l':
               opt_set[opt_l]++;
               n = sscanf(optarg, "%d:%d",
                          &opt->decryptopt.fromlen,
                          &opt->decryptopt.tolen);

               switch (n) {
               case 0:         /* option --length used with no values */
                    usage(ERR_USAGE);
                    break;
               case 1:         /* only 'fromlen' set */
                    opt->decryptopt.tolen = opt->decryptopt.fromlen;
               case 2:
                    if (opt->decryptopt.fromlen >
                        opt->decryptopt.tolen)
                         tty_error(ERR_USAGE,
                                   "bad order for `--length' limits");
                    if (opt->decryptopt.fromlen < 1)
                         tty_error(ERR_USAGE,
                                   "illegal downlimit for `--length'");
                    if (opt->decryptopt.tolen > MAX_PLAIN_SIZE)
                         tty_error(ERR_USAGE,
                                   "the `--length' upperlimit exceed `%d'",
                                   MAX_PLAIN_SIZE);
               }
               break;
          case 'b':            /* try the brute-force attack */
               opt_set[opt_b]++;
               opt->action = decrypt_bf;
               if (optarg)
                    opt->decryptopt.regexpr = str_alloc_copy(optarg);
               break;
          case 'w':            /* try the vocabulary attack */
               opt_set[opt_w]++;
               opt->action = decrypt_wl;
               opt->decryptopt.wordlist = str_alloc_copy(optarg);
               break;
#if defined (HAVE_WORKING_FORK)
          case 'D':
               opt_set[opt_D]++;
               opt->daemonize = 1;
               break;
#endif
          case 'r':
               opt_set[opt_r]++;
               opt->action = resume;
               /* default rescue file is `RESTORE_FILE' */
               opt->decryptopt.restore_file = optarg ?
                    str_alloc_copy(optarg) : str_alloc_copy(RESTORE_FILE);
               break;
          case 'q':            /* minimize the program output */
               opt_set[opt_q]++;
               opt->verbose = 0;
               break;
          case 'h':            /* help */
#if 0
               opt_set[opt_h]++;
#endif
               usage(SUCCESS);
               break;
          case 'v':            /* verbose mode */
               opt_set[opt_v]++;
               opt->verbose++;
               break;
          case 'L':
#if 0
               opt_set[opt_L]++;
#endif
               license();      /* GPL license message */
               exit(SUCCESS);
               break;
          case 'V':            /* program name, version, etc. */
#if 0
               opt_set[opt_V]++;
#endif
               version();
               exit(SUCCESS);
               break;
          case '?':
               tty_error(ERR_USAGE,
                         "ambiguous match or extraneous parameter `%s'",
                         argv[optind - 1]);
               break;
          default:   /* should catch nothing (?) */
               tty_error(ERR_USAGE, "illegal command line argument");
          }
     }

     /* check for option inconsistences */
     opt_check(opt_set);
}

typedef enum e_bool { false = 0, true } bool;
extern bool save_session_if_signal_abort;

t_options usr_opt;

int
main(int argc, char **argv)
{
     int i, retval = SUCCESS;
#if defined (HAVE_WORKING_FORK)
     pid_t cpid;
     FILE *fscore;
#endif

#if defined (HAVE_ASSERT_H) && defined (SHORT_PASSWORDS)
     assert(MAX_PLAIN_SIZE < 16);
#endif

     /* set the default values for `usr_opt' */
     memset(&usr_opt, 0, sizeof(usr_opt));
     usr_opt.verbose = 1;
#if defined (DEV_RANDOM_SUPPORT)
     usr_opt.cryptopt.get_entropy = get_dev_entropy;
#elif defined (EGD_SUPPORT)
     usr_opt.cryptopt.get_entropy = get_egd_entropy;
#else
     usr_opt.cryptopt.get_entropy = get_sys_entropy;
#endif

     opt_parse(argc, argv, &usr_opt);

#if defined (HAVE_UMASK)
     /* set umask for security reasons */
     umask(0077);
#endif

     switch (usr_opt.action) {
     case crypt_ios:
     case crypt_pix:
          if (argc != optind + 1)
               tty_error(ERR_USAGE, "no string to crypt");
          usr_opt.cryptopt.plain = (u_char *) str_alloc_copy(argv[optind]);

          retval = (usr_opt.action == crypt_ios) ?
                    crypt_ios_fe(usr_opt.cryptopt.plain,
                                 usr_opt.cryptopt.salt) :
                    crypt_pix_fe(usr_opt.cryptopt.plain);
          break;
     case decrypt_bf:
     case decrypt_wl:
          if (argc != optind + 1)
               tty_error(ERR_USAGE, "no password to decrypt");
          usr_opt.decryptopt.cipher = (u_char *) str_alloc_copy(argv[optind]);
          /* no break here! */
     case resume:
          /* if `decrypt_bf', `decrypt_wl', `resume_bf', `resume_wl',
           * create the restore file in case of an abort event
           */
          save_session_if_signal_abort = true;

          if (usr_opt.action == resume) {
               if (argc != optind)
                    tty_error(ERR_USAGE,
                              "bad argument found `%s'", argv[optind]);

               session_load(usr_opt.decryptopt.restore_file);

               /* `usr_opt.decryptopt.regexpr' can be NULL,
                *  so we must check `usr_opt.decryptopt.wordlist' */
               usr_opt.action =
                    usr_opt.decryptopt.wordlist ? resume_wl: resume_bf;
          }

#if defined (HAVE_WORKING_FORK)
          if (usr_opt.daemonize) {
               cpid = fork();
               if (cpid == (pid_t) 0) {   /* child process */
                    /* checks the SCORE_FILE format */
                    fcheck(SCORE_FILE, HEAD_SCORE_TEXT);

                    /* add the line `HEAD_SCORE_TEXT"\n"' at the begin,
                     * if the file do not exists */
                    if (!(fscore = fopen(SCORE_FILE, "r"))) {
                         fscore = fopen(SCORE_FILE, "w");
                         fprintf(fscore, HEAD_SCORE_TEXT"\n");
                    }
                    fclose(fscore);
                    tty_message
                        ("%s -- running in daemon mode"
#if defined (HAVE_GETPID)
                         " [pid: %d]"
#endif
                         "...\n** WARNING: "
                         "sensible data could be written in the file `%s'!\n",
                         *argv,
#if defined (HAVE_GETPID)
                         (int) getpid(),
#endif
                         SCORE_FILE);
                    /* point stdin/stdout/stderr to /dev/null */
                    for (i = 0; i < 3; i++)
                         close(i);
                    i = open(NULL_DEV, O_RDWR);
                    if (i >= 0) {
                         dup2(i, 0);
                         dup2(i, 1);
                         dup2(i, 2);
                         if (i > 2)
                              close(i);
                    }
                    retval = cisco_decrypt();
                    return retval;
               } else if (cpid < (pid_t) 0)
                    tty_error(ERR_FORKING,
                              "cannot run in daemon mode : %s",
                              strerror(errno));
               else
                    return SUCCESS;     /* parent process */
          } else
#endif
               retval = cisco_decrypt();
          break;
     default:                  /* not enought paramethers entered */
          tty_error(ERR_USAGE, "nothing to do (?)");
     }

     mem_free_all();
     return retval;
}
