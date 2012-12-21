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

#include <ctype.h>       /* `isspace', `isdigit', `isalnum' */
#include <stdarg.h>      /* `va_list' */
#include <stdio.h>
#if STDC_HEADERS
# include <stdlib.h>     /* exit */
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
#if HAVE_SYS_STAT_H
# include <sys/stat.h>
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

#include "decrypt.h"
#include "main.h"
#include "mem.h"
#include "params.h"
#include "session.h"
#include "tty.h"

typedef enum e_bool { false = 0, true } bool;

/* *INDENT-OFF* */
typedef enum t_dfa_state_code {   /* possible states of the dfa */
     s_start,
     s_comment,                   /* `;' any character sequence<eol> */
     s_backslash,
     s_keyword,
     s_number,
     s_string                     /* "sequence of chars" */
} t_dfa_state_code;

typedef enum t_dfa_error_code {
     e_none = 0,
     e_unknown_token,
     e_unknown_keyword,
     e_unterminated_keyword_name,
     e_unterminated_string,
     e_number_expected,
     e_int_exceed,
     e_string_exceed,
     e_eof
} t_dfa_error_code;

#define DFA_STRING_SIZE   128
#define DFA_MAXINT       1024

typedef struct t_token_data {
     char ch;
     long int number;
     char str[DFA_STRING_SIZE + 1];
} t_token_data;

typedef enum t_token_code {
     t_none = 0,
     t_char, t_number, t_string,
     t_open_square_bracket, t_close_square_bracket,
     t_ids,               /* startpoint of the list of identifiers */
     t_chpos = t_ids,
     t_cipher,
     t_date,
     t_method,
     t_range,
     t_regex,
     t_seekpoint,
     t_verbose,
     t_wordlist,
     t_eof
} t_token_code;
#define KEYWORD_NUM  (int)(t_wordlist - t_ids + 1)
/* *INDENT-ON* */

typedef struct t_keyword_id {
     const char *id;
     t_token_code t_code;
} t_keyword_id;

t_keyword_id keyword[] = {
     { "chpos",     t_chpos     },
     { "cipher",    t_cipher    },
     { "date",      t_date      },
     { "method",    t_method    },
     { "range",     t_range     },
     { "regex",     t_regex     },
     { "seekpoint", t_seekpoint },
     { "verbose",   t_verbose   },
     { "wordlist",  t_wordlist  },
     { 0,           t_none      }
};
/* *INDENT-ON* */

static t_token_code lookahead_code;
static t_token_data *token_data;
static t_dfa_error_code dfa_error_code;
static char *cp, *cp_err, *confline;

char seekpoint[FLINE_BUFSIZE];

extern int *chpos, *chpos_safe, actual_len;
extern char wbuf[FLINE_BUFSIZE];
extern t_options usr_opt;

typedef enum t_usr_opt_method {
     is_not_set = 0,
     is_set, unused,
     bruteforce,
     wordlist
} t_usr_opt_method;

static void dfa_init(char *in_confline, t_token_data *token);
static t_token_code dfa_get_token(void);
static void dfa_match(t_token_code token_code);
static int dfa_message(const char *fmt, ...);

static void
dfa_init(char *in_confline, t_token_data *token)
{
     cp = cp_err = confline = in_confline;
     token_data = token;
}

static t_token_code
dfa_get_token(void)
{
     bool exit_flag = false, read_flag = true;
     char ch;
     u_int i, string_level = 0;
     t_dfa_state_code dfa_state = s_start, dfa_substate = s_start;
     t_token_code token_code = t_none;

     dfa_error_code = e_none;
     ch = *cp;
     cp_err = cp;      /* `cp_err' points to the begin of the token */

     do {
          switch (dfa_state) {
          case s_start:
               if (!ch) {
                    token_code = t_eof;
                    exit_flag = true;
               } else if (ch == '\n' || isspace(ch)) {
                    /* just ignore the newline character */
                    cp_err++;
                    read_flag = true;
               } else if (ch == ';') {
                    dfa_state = s_comment;
                    read_flag = true;
               } else if (isdigit(ch)) {
                    dfa_state = s_number;
                    token_data->number = (long int)(ch - '0');
                    read_flag = true;
               } else if (ch == '"') {
                    dfa_state = dfa_substate = s_string;
                    read_flag = true;
               } else if (ch == '[') {
                    token_code = t_open_square_bracket;
                    exit_flag = true;
               } else if (ch == ']') {
                    token_code = t_close_square_bracket;
                    exit_flag = true;
               } else {
                    dfa_state = s_keyword;
                    string_level = 0;
                    read_flag = false;
               }
               break;
          case s_comment:
               if (ch == '\n')
                    dfa_state = s_start;
               else if(!ch) { /* eof */
                    token_code = t_none;
                    dfa_error_code = e_eof;
                    exit_flag = true;
               }
               break;
          case s_keyword:
               if (string_level > DFA_STRING_SIZE) {
                    cp_err++;
                    dfa_error_code = e_string_exceed;
                    read_flag = false; exit_flag = true;
               }
               else if (isalnum((int)ch)) {
                    token_data->str[string_level++] = ch;
                    read_flag = true;
               }
               else if (!ch || ch == '\n') {   /* eol/eof in the string */
                    cp_err++;
                    dfa_error_code = e_unterminated_keyword_name;
                    read_flag = false; exit_flag = true;
               }
               else {                         /* end of keyword */
                    token_data->str[string_level] = '\0';
                    token_code = t_none;
                    for (i = 0; keyword[i].id; i++) {
                         if (!strcmp(token_data->str, keyword[i].id)) {
                              token_code = keyword[i].t_code;
                              break;
                         }
                    }
                    if (token_code != t_none)
                        read_flag = true;
                    else {
                         cp_err++;
                         dfa_error_code = e_unknown_keyword;
                         read_flag = false;
                    }
                    exit_flag = true;
               }
               break;
          case s_number:
               if(isdigit(ch)) {
                    token_data->number =
                         token_data->number * 10 + (long int)(ch - '0');
                    if (token_data->number > DFA_MAXINT)
                         dfa_error_code = e_int_exceed;
                    else
                         read_flag = true;
               }
               else {
                    token_code = t_number;
                    read_flag = false;
                    exit_flag = true;
               }
               break;
          case s_string:
               if (string_level > DFA_STRING_SIZE) {
                    cp_err++;
                    dfa_error_code = e_string_exceed;
                    read_flag = false; exit_flag = true;
               }
               else if (ch == '\\') {
                    dfa_substate =
                        (dfa_substate == s_backslash) ? s_string : s_backslash;
                    token_data->str[string_level++] = ch;
               } else if (ch == '"') {  /* end of string */
                    if (dfa_substate != s_backslash) {
                         token_code = t_string;
                         token_data->str[string_level] = '\0';
                         exit_flag = true;
                    }
                    /* found `\"', or `\\\"', or so on...,
                     * it's an escaped `"', not an end of string character
                     */
                    else {
                         cp_err++;
                         dfa_substate = s_string;
                         token_data->str[string_level++] = ch;
                    }
               } else if (!ch || ch == '\n') {
                    /* eol/eof in the string */
                    cp_err++;
                    dfa_error_code = e_unterminated_string;
                    read_flag = false; exit_flag = true;
               }
               else {
                    dfa_substate = s_string;
                    token_data->str[string_level++] = ch;
               }
               break;
          default:
               token_code = t_none;
               dfa_error_code = e_unknown_token;
               break;
          }
          if (read_flag)
               ch = *(++cp);
     } while (!exit_flag);

     switch (dfa_error_code) {
          case e_none:
               break;
          case e_unknown_token:
               dfa_message("illegal input");
               break;
          case e_unknown_keyword:
               dfa_message("unknown keyword");
               break;
          case e_unterminated_keyword_name:
               dfa_message("unterminated keyword name");
               break;
          case e_unterminated_string:
               dfa_message("unterminated string");
               break;
          case e_number_expected:
               dfa_message("positive integer expected");
               break;
          case e_int_exceed:
               dfa_message("integer size exceed maximum allowed");
               break;
          case e_string_exceed:
               dfa_message("string length exceed buffer size");
               break;
          case e_eof:
               dfa_message("unexpected end of file found");
               break;
          default:
               dfa_message("%s : bug in %s", __FILE__, __FUNCTION__);
     }

     return token_code;
}

static void
dfa_match(t_token_code token_code)
{
     lookahead_code = dfa_get_token();
     if (lookahead_code != token_code) {
          switch (token_code) {
          case t_open_square_bracket:
               dfa_message("`[' expected");
               break;
          case t_close_square_bracket:
               dfa_message("`]' expected");
               break;
          case t_number:
               dfa_message("number expected");
               break;
          case t_string:
               dfa_message("string expected");
               break;
          default:             /* should never be matched */
               dfa_message("%s : bug in %s", __FILE__, __FUNCTION__);
          }
     }
}

static int
dfa_message(const char *fmt, ...)
{
     va_list argptr;
     int i;

#if 0
     /* no messages in quiet mode, only return codes */
     if (usr_opt.verbose)
          exit(ERR_PARSER);
#endif

     fflush(stdout);            /* to avoid some visualisation problems */
     fprintf(stderr, "\n** ERROR: buggy restore file\n%s", confline);

     for (i = 0; i < cp_err - confline; i++)
          fprintf(stderr, " ");
     fprintf(stderr, "^ ");

     va_start(argptr, fmt);
     vfprintf(stderr, fmt, argptr);
     va_end(argptr);

     fprintf(stderr,
             "\n\nenter `%s --help' if you need help\n\n", PACKAGE);
     exit(ERR_PARSER);
}

void
session_save(void)
{
     FILE *frestore;
     int i, len;
     time_t ltime;
     char *time_str;

     /* checks the RESTORE_FILE format */
     fcheck(RESTORE_FILE, HEAD_RESTORE_TEXT);

     if (!(frestore = fopen(RESTORE_FILE, "w")))
          tty_error(ERR_IO_FILE, "cannot create the file `%s'", RESTORE_FILE);

     if (usr_opt.decryptopt.regexpr) {
          fprintf(frestore,
                  HEAD_RESTORE_TEXT"\n"
                  "method \"bruteforce\"\n"
                  "range [%d %d %d]\n",
                  usr_opt.decryptopt.fromlen, usr_opt.decryptopt.tolen,
                  actual_len);

          fprintf(frestore, "chpos [");
          for (i = 0; i < usr_opt.decryptopt.tolen - 1; i++)
               fprintf(frestore, "%d ", chpos_safe[i]);

          fprintf(frestore,
                  "%d]\n"
                  "regex \"%s\"\n",
                  chpos_safe[usr_opt.decryptopt.tolen - 1],
                  usr_opt.decryptopt.regexpr);
     }
     else if (usr_opt.decryptopt.wordlist) {
          fprintf(frestore,
                  HEAD_RESTORE_TEXT"\n"
                  "method \"wordlist\"\n"
                  "range [%d %d]\n"
                  "wordlist \"%s\"\n",
                  usr_opt.decryptopt.fromlen, usr_opt.decryptopt.tolen,
                  usr_opt.decryptopt.wordlist);
          if (wbuf[(len = strlen(wbuf)) - 1] == '\n')
               wbuf[--len] = '\0';
          fprintf(frestore, "seekpoint \"%s\"\n", wbuf);
     }
#if 0
     else
          tty_error(INTERNAL, "%s::%s -- internal error",
                    __FILE__, __FUNCTION__);
#endif

     fprintf(frestore,
             "cipher \"%s\"\n"
             "verbose %d\n",
             usr_opt.decryptopt.cipher, usr_opt.verbose);

     time(&ltime);

     time_str = str_alloc_copy(ctime(&ltime));
     time_str[strlen(time_str) - 1] = '\0';
     fprintf(frestore, "date \"%s\"\n", time_str);
     mem_free(time_str);

     fclose(frestore);
}

void
session_load(char *rescue_file)
{
     t_usr_opt_method load_opt[KEYWORD_NUM];
     t_token_data token;
     char buf[FLINE_BUFSIZE], *s;
     int i, line_num = 0;
     FILE *frescue;

     if (!(frescue = fopen(rescue_file, "r")))
          tty_error(ERR_IO_FILE, "cannot found the file `%s'", rescue_file);

     for (i = 0; i < KEYWORD_NUM; i++)
          load_opt[i] = is_not_set;
     usr_opt.action = none;

     while ((s = fgets(buf, FLINE_BUFSIZE, frescue))) {
          dfa_init(s, &token);
          lookahead_code = dfa_get_token();
          line_num++;
          while (lookahead_code != t_none && lookahead_code != t_eof) {
               switch (lookahead_code) {
               case t_method:
                    dfa_match(t_string);
                    if (!strcmp(token.str, "bruteforce")) {
                         usr_opt.action = decrypt_bf;
                         load_opt[t_wordlist - t_ids] = unused;
                         load_opt[t_seekpoint - t_ids] = unused;
                    }
                    else if (!strcmp(token.str, "wordlist")) {
                         usr_opt.action = decrypt_wl;
                         load_opt[t_regex - t_ids] = unused;
                         load_opt[t_chpos - t_ids] = unused;
                    }
                    else
                         dfa_message("unknown method `%s'", token.str);
                    load_opt[t_method - t_ids] = is_set;
                    break;
               case t_range:
                    if (usr_opt.action == none)
                         dfa_message("`method' must be set before `range'");

                    dfa_match(t_open_square_bracket);
                    dfa_match(t_number);
                    usr_opt.decryptopt.fromlen = token.number;
                    if (token.number < 0 || token.number > MAX_PLAIN_SIZE)
                         dfa_message("illegal or too big value");

                    dfa_match(t_number);
                    usr_opt.decryptopt.tolen = token.number;
                    if (token.number < 0 ||
                        token.number > MAX_PLAIN_SIZE ||
                        token.number < usr_opt.decryptopt.fromlen)
                         dfa_message("illegal or too big value");
                    /* the `wordlist' method does not require `actual_len' */
                    if (usr_opt.action == decrypt_bf) {
                         dfa_match(t_number);
                         actual_len = token.number;
                         if (actual_len < usr_opt.decryptopt.fromlen ||
                             actual_len > usr_opt.decryptopt.tolen)
                              dfa_message("illegal or too big value");
                    }
                    dfa_match(t_close_square_bracket);
                    load_opt[t_range - t_ids] = is_set;
                    break;
               case t_regex:
                    dfa_match(t_string);
                    usr_opt.decryptopt.regexpr = str_alloc_copy(token.str);
                    load_opt[t_regex - t_ids] = is_set;
                    break;
               case t_chpos:
                    if (!load_opt[t_range - t_ids])
                         dfa_message("`range' must be set before `chpos'");

                    dfa_match(t_open_square_bracket);

                    chpos = zmem_alloc(MAX_PLAIN_SIZE * sizeof(*chpos));
                    for (i = 0; i < usr_opt.decryptopt.tolen; i++) {
                         dfa_match(t_number);
                         if ((chpos[i] = token.number) < 0)
                              dfa_message("illegal value for chpos");
                    }
                    dfa_match(t_close_square_bracket);
                    load_opt[t_chpos - t_ids] = is_set;
                    break;
               case t_wordlist:
                    dfa_match(t_string);
                    usr_opt.decryptopt.wordlist = str_alloc_copy(token.str);
                    load_opt[t_wordlist - t_ids] = is_set;
                    break;
               case t_seekpoint:
                    dfa_match(t_string);
                    strncpy(seekpoint, token.str, MAX_PLAIN_SIZE);
                    /* if case someone has modified the restore file... */
                    seekpoint[MAX_PLAIN_SIZE] = '\0';
                    load_opt[t_seekpoint - t_ids] = is_set;
                    break;
               case t_cipher:
                    dfa_match(t_string);
                    usr_opt.decryptopt.cipher =
                        (u_char *) str_alloc_copy(token.str);
                    load_opt[t_cipher - t_ids] = is_set;
                    break;
               case t_verbose:
                    dfa_match(t_number);
                    if ((usr_opt.verbose = token.number) < 0)
                         dfa_message("illegal value for `verbose'");
                    load_opt[t_verbose - t_ids] = is_set;
                    break;
               case t_date:
                    dfa_match(t_string);
                    usr_opt.decryptopt.saved_date = str_alloc_copy(token.str);
                    load_opt[t_date - t_ids] = is_set;
                    break;
               default:
                    tty_error(ERR_INFO_RFILE,
                              "not a session file `%s' (line %d)",
                              rescue_file, line_num);
               }
               lookahead_code = dfa_get_token();
          }
     }
     fclose(frescue);

     for (i = 0; i < KEYWORD_NUM; i++)
          if (load_opt[i] == is_not_set)
               tty_error(ERR_INFO_RFILE,
                         "mandatory option `%s' not found in `%s'",
                         keyword[i].id, rescue_file);

     if (usr_opt.decryptopt.regexpr && usr_opt.decryptopt.wordlist)
          tty_error(ERR_INFO_RFILE,
                    "incoherent options found in `%s'", rescue_file);
}

/* check the SCORE_FILE format (not to overwrite user/system files via a
 * symlink attack)
 * On Linux and FreeBSD the `open' flags `O_NOFOLLOW' and `O_EXCL' can be
 * used instead
 */
void
fcheck(const char *fname, const char *checkline)
{
     u_int buf_size = strlen(checkline) + 1;
     char *buf;
     FILE *f2check;

     buf = mem_alloc(buf_size);
     if ((f2check = fopen(fname, "r"))) {   /* the file exists */
          fgets(buf, buf_size, f2check);
          fprintf(stderr, "\nthe file `%s' already exists..."
                 "\nsecurity check... ", fname);
          if (strncmp(buf, checkline, buf_size - 2)) {
               fprintf(stderr, "failed!\n** ERROR: invalid file!\n");
               fclose(f2check);
               mem_free(buf);
               exit(ERR_INFO_RFILE);
          }
          else {
               fprintf(stderr, "passed.\n");
               fclose(f2check);
          }
     }
#if 0
     else {
          if (!(f2check = fopen(fname, "w")))
               tty_error(ERR_IO_FILE,
                         "cannot create the file `%s'", fname);

          fprintf(f2check, "%s\n", checkline);
          fclose(f2check);
     }
#endif
     mem_free(buf);
}
