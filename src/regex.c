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

#include <ctype.h>       /* `isalpha' */
#include <stdarg.h>      /* `va_list' */
#include <stdio.h>       /* `fflush', `stdout', `stderr' */
#if STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# if HAVE_STDLIB_H
#  include <stdlib.h>    /* `strtol' */
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
#if !HAVE_STRCHR
# define strchr index
#endif

#include "mem.h"
#include "params.h"      /* `MAX_PLAIN_SIZE' */
#include "regex.h"
#include "tty.h"

typedef enum t_bool { false = 0, true } bool;

/* *INDENT-OFF* */
typedef enum t_dfa_state_code {   /* possible states of the dfa */
     s_start,
     s_backslash,
     s_alphabet,                  /* `:'<alphabet_id>`:' */
     s_number                     /* `<number>' */
} t_dfa_state_code;

typedef enum t_dfa_error_code {   /* same order as in `dfa_error_msg[]' */
     e_none = 0,
     e_illegal_character,
     e_illegal_token,
     e_unknown_charset,
     e_unterminated_charset_name,
     e_number_expected,
     e_close_curly_brace_expected,
     e_buf_full_charset
} t_dfa_error_code;

#define TDSTR_BUFSIZE  128
typedef struct t_token_data {
     char ch;
     long int number;
     char str[TDSTR_BUFSIZE + 1];
} t_token_data;

typedef enum t_token_code {
     t_none = 0,
     t_char, t_number,
     t_caret /*^*/, t_hyphen /*-*/, t_period /*.*/,
     t_open_square_bracket, t_close_square_bracket,
     t_alphabet,             /* startpoint for the list of alphabets */
     t_all = t_alphabet, t_alnum, t_alpha, t_digit, t_lower, t_upper,
     t_eof
} t_token_code;

typedef struct t_keyword_id {
     const char *id;
     t_token_code token_code;
} t_keyword_id;

static t_keyword_id charset[] = {
     { "all",    t_all   },
     { "alnum",  t_alnum },
     { "alpha",  t_alpha },
     { "digit",  t_digit },
     { "lower",  t_lower },
     { "upper",  t_upper },
     { 0,        t_none  }
};

static t_keyword_id separator[] = {
     { "[",      t_open_square_bracket  },
     { "]",      t_close_square_bracket },
     { 0  ,      t_none                 }
};

/* *INDENT-ON* */

extern u_char md5_magic[];
extern u_char md5_itoa64[];
extern u_char cisco_alphabet[];

static t_token_code lookahead_code;
static t_token_data *token_data;
static t_dfa_error_code dfa_error_code;
static char *cp, *cp_err, *regexp;
static int letters;

static void dfa_init(char *in_str, t_token_data *token);
static t_token_code dfa_get_token(void);
static void dfa_match(t_token_code token_code);
static int dfa_message(const char *fmt, ...);

static void set_char_list(char * ch_list, t_token_data * token);

static void
dfa_init(char *in_regexp, t_token_data *token)
{
     cp = cp_err = regexp = in_regexp;
     token_data = token;
}

t_token_code
dfa_get_token(void)
{
     bool exit_flag = false, read_flag = true;
     char ch, *ep;
     u_int i, string_level = 0;
     t_dfa_state_code dfa_state = s_start;
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
               } else if (ch == '\\') {
                    dfa_state = s_backslash;
                    read_flag = true;
               } else if (ch == '{') {
                    dfa_state = s_number;
                    read_flag = true;
               } else if (ch == '[') {
                    token_code = t_open_square_bracket;
                    exit_flag = true;
               } else if (ch == ']') {
                    token_code = t_close_square_bracket;
                    exit_flag = true;
               } else if (ch == '^') {
                    token_code = t_caret;
                    exit_flag = true;
               } else if (ch == '-') {
                    token_code = t_hyphen;
                    exit_flag = true;
               } else if (ch == '.') {
                    token_code = t_period;
                    exit_flag = true;
               } else if (ch == ':') {
                    dfa_state = s_alphabet;
                    string_level = 0;
                    /* discard the starting `:' char */
                    read_flag = true;
               } else if (!strchr((char *)cisco_alphabet, ch)) {
                    token_code = t_none;
                    dfa_error_code = e_illegal_character;
                    exit_flag = true;
               } else {
                    token_code = t_char;
                    token_data->ch = ch;
                    exit_flag = true;
               }
               break;
          case s_backslash:
               token_code = t_char;
               token_data->ch = ch;
               exit_flag = true;
               break;
          case s_alphabet:
               if (string_level > TDSTR_BUFSIZE) {
                    cp_err++;
                    dfa_error_code = e_buf_full_charset;
                    read_flag = false; exit_flag = true;
               }
               else if (ch == ':') {    /* end of alphabet id */
                    token_data->str[string_level] = '\0';
                    token_code = t_none;
                    for (i = 0; charset[i].id; i++) {
                         if (!strcmp(token_data->str, charset[i].id)) {
                              token_code = charset[i].token_code;
                              break;
                         }
                    }
                    if (token_code == t_none) {
                         cp_err++;
                         dfa_error_code = e_unknown_charset;
                         read_flag = false; exit_flag = true;
                    }
                    read_flag = exit_flag = true;
               }
               /* eol/illegal-char in the string */
               else if (!ch || ch == '\n' || !isalpha((int)ch)) {
                    cp_err++;
                    dfa_error_code = e_unterminated_charset_name;
                    read_flag = false; exit_flag = true;
               }
               else
                    token_data->str[string_level++] = ch;
               break;
          case s_number:
               token_code = t_number;
               token_data->number = strtol(cp, &ep, 10);
               if (cp == ep) {
                    cp_err = cp;
                    dfa_error_code = e_number_expected;
               }
               else if (*(cp = ep) != '}') {
                    cp_err = cp;
                    dfa_error_code = e_close_curly_brace_expected;
               }
               cp++;
               read_flag = false;
               exit_flag = true;
               break;
          /* the `default' label should never be reached */
          default:
               token_code = t_none;
               dfa_error_code = e_illegal_token;
               break;
          }
          if (read_flag)
               ch = *(++cp);
     } while (!exit_flag);

     switch (dfa_error_code) {
          case e_none:
               break;
          case e_illegal_character:
               dfa_message("illegal character");
               break;
          case e_illegal_token:
               dfa_message("illegal regular expression");
               break;
          case e_unknown_charset:
               dfa_message("unknown character set id");
               break;
          case e_unterminated_charset_name:
               dfa_message("unterminated character set name");
               break;
          case e_number_expected:
               dfa_message("positive integer expected");
               break;
          case e_close_curly_brace_expected:
               dfa_message("close curly brace `}' expected");
               break;
          case e_buf_full_charset:
               dfa_message("name too long for a charset");
               break;
          default:
               dfa_message("%s : bug in %s", __FILE__, __FUNCTION__);
     }

     return token_code;
}

static void
dfa_match(t_token_code token_code)
{
     u_int i;

     lookahead_code = dfa_get_token();
     if (lookahead_code != token_code) {
          for (i = 0; separator[i].token_code; i++)
               if (separator[i].token_code == token_code)
                    dfa_message("expected token `%s'", separator[i].id);

          /* should never be reached (see `separator[]') */
          dfa_message("%s : bug in %s", __FILE__, __FUNCTION__);
     }
}

static int
dfa_message(const char *fmt, ...)
{
     va_list argptr;
     int i;

     fflush(stdout);   /* to avoid some visualisation problems */
     fprintf(stderr, "\n** ERROR: buggy regular expression\n%s\n", regexp);
     for (i = 0; i < cp_err - regexp; i++)
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
set_char_list(char * ch_list, t_token_data * token)
{
     int i, alph_len, ch, delta, letters_before;
     bool parser_err = true, read_flag = true;

     while (lookahead_code != t_close_square_bracket) {
          parser_err = read_flag = true;
          /* note :
           *  bash:~> echo 1 | grep [^[:digit:][:digit:]]
           *  bash:~>
           *  bash:~> grep -V | head -1
           *  bash:~> grep (grep GNU) 2.4.2
           */
          if (lookahead_code == t_caret) {
               dfa_match(t_open_square_bracket);
               set_char_list(ch_list, token);
               alph_len = strlen((char *)cisco_alphabet);
               /* move the letters at the end of `ch_list' */
               for (i = letters - 1; i >= 0; i--) {
                    ch_list[alph_len - letters + i] = ch_list[i];
                    ch_list[i] = '\0';
               }
               letters_before = letters;
               /* build the complementary vector */
               for (letters = 0, i = 0; i < alph_len; i++)
                    if (!strchr(ch_list + alph_len - letters_before,
                        (char)cisco_alphabet[i]))
                         ch_list[letters++] = (char)cisco_alphabet[i];

               for (i = alph_len - letters_before; i < alph_len; i++)
                    ch_list[i] = '\0';
               read_flag = parser_err = false;
          }
          if (lookahead_code == t_open_square_bracket) {
               lookahead_code = dfa_get_token();
               set_char_list(ch_list, token);
               parser_err = false;
          }
          if (lookahead_code == t_all) {
               /* `[:all:]' means all the char set */
               strcpy(ch_list, (char *)cisco_alphabet);
               letters = strlen((char *)cisco_alphabet);
               parser_err = false;
          }
          if (lookahead_code == t_lower || lookahead_code == t_alpha ||
              lookahead_code == t_alnum) {
               /* `[:lower:]' means `[a-z]' */
               /* `[:alpha:]' means `[A-Za-z]' */
               for (i = 'a'; i <= 'z'; i++)
                    /* skip duplicate letters */
                    if (!strchr(ch_list, (char)i))
                         ch_list[letters++] = (char)i;
               parser_err = false;
          }
          if (lookahead_code == t_upper || lookahead_code == t_alpha ||
              lookahead_code == t_alnum) {
               /* `[:upper:]' means `[A-Z]' */
               for (i = 'A'; i <= 'Z'; i++)
                    if (!strchr(ch_list, (char)i))
                         ch_list[letters++] = (char)i;
               parser_err = false;
          }
          if (lookahead_code == t_digit || lookahead_code == t_alnum) {
               /* `[:digit:]' means `[0-9]' */
               for (i = '0'; i <= '9'; i++)
                    /* skip duplicate letters */
                    if (!strchr(ch_list, (char)i))
                         ch_list[letters++] = (char)i;
               parser_err = false;
          }
          if (lookahead_code == t_char) {
               ch = token->ch;
               delta = 0;
               lookahead_code = dfa_get_token();
               if (lookahead_code == t_hyphen) {
                    lookahead_code = dfa_get_token();
                    if (lookahead_code != t_char)
                         dfa_message("single character expected");
                    else if ((delta = token->ch - ch) < 0)
                         dfa_message("sequence of chars expected");
                    read_flag = true;
               }
               else
                    read_flag = false;
               for (i = 0; i <= delta; i++)
                    /* skip duplicate letters */
                    if (!strchr(ch_list, ch + (char)i))
                         ch_list[letters++] = ch + (char)i;
               parser_err = false;
          }

          if (parser_err)
               (lookahead_code == t_eof) ?
                    dfa_message("missing close square bracket `]'"):
                    dfa_message("illegal regular expression");

          if (read_flag)
               lookahead_code = dfa_get_token();
     }
}

void
regex_explode(char **alphabet, char *regexpr, int *fromlen, int *tolen)
{
     bool read_flag = true;
     int i, arg = 0;
     long int ndup;
     t_token_data token;
     static char *buf = NULL, *ch_list = NULL;

     ch_list = mem_alloc(strlen((char *)cisco_alphabet) + 1);

     dfa_init(regexpr, &token);

     lookahead_code = dfa_get_token();
     while (lookahead_code != t_eof) {
          switch (lookahead_code) {
          case t_char:
               /* a single character */
               alphabet[arg] = zmem_alloc(2);
               alphabet[arg][0] = token.ch;
               arg++;
               break;
          case t_period:
               /* a period means the all the alphabet */
               alphabet[arg] = str_alloc_copy((char *)cisco_alphabet);
               arg++;
               break;
          case t_open_square_bracket:
               lookahead_code = dfa_get_token();
               letters = 0;
               memset(ch_list, 0, strlen((char *)cisco_alphabet) + 1);
               set_char_list(ch_list, &token);
               alphabet[arg++] = str_alloc_copy(ch_list);
               lookahead_code = dfa_get_token();
               /* `alphabet' must be replicated */
               if (lookahead_code == t_number) {
                    ndup = token.number;
                    if (ndup < 1 || (arg + ndup - 1 > MAX_PLAIN_SIZE))
                         dfa_message("illegal or too big multiplier");
                    /* duplicate the previous alphabet */
                    for (i = 0; i < ndup - 1; i++)
                         alphabet[arg + i] = alphabet[arg - 1];
                    arg += (ndup - 1);
               }
               else
                    read_flag = false;
               break;
          default:
               dfa_message("not a valid regular string");
               break;
          }
          read_flag ? (lookahead_code = dfa_get_token()) : (read_flag = true);
     }

     mem_free(buf);
     mem_free(ch_list);

     if (arg > MAX_PLAIN_SIZE)
          tty_error(ERR_USAGE,
                    "regexpr too long (%d elements max)", MAX_PLAIN_SIZE);

     if (!*fromlen)   /* `fromlen' not set */
          *fromlen = arg > 1 ? arg : 1;
     if (!*tolen)     /* `tolen' not set */
          *tolen = arg > 1 ? arg : MAX_PLAIN_SIZE;

     if (arg == 1) {            /* option -b with no regex entered by user */
          for (i = 1; i < MAX_PLAIN_SIZE; i++)  /* duplicate `alphabet[0]' */
               alphabet[i] = alphabet[0];
     } else if (arg < *tolen) {
          tty_warning("max length reduced to match the regexpr length");
          *tolen = arg;
          if (*fromlen > arg)
               *fromlen = arg;
     } else if (arg > *tolen) {
          tty_warning("max length incremented to match regexpr length");
          *tolen = arg;
     }
}
