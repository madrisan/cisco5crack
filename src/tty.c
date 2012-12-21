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

#include <fcntl.h>              /* open, O_RDONLY, O_NONBLOCK */
#include <signal.h>
#if STDC_HEADERS
# include <stdarg.h>
#endif
#include <stdio.h>
#if HAVE_STDLIB_H
# include <stdlib.h>
#endif
#ifdef HAVE_TERMIOS_H
# include <termios.h>           /* struct termios, tcsetattr */
# if HAVE_UNISTD_H
#  include <unistd.h>           /* close */
# endif
#endif

#include "main.h"
#include "mem.h"
#include "session.h"
#include "signals.h"
#include "tty.h"

#if defined(HAVE_TERMIOS_H) && defined(HAVE_DEV_TTY)
int tty_fd = 0;
static struct termios tty_saved_attr;
#endif

extern t_options usr_opt;

extern void
tty_init(void)
{
#if defined(HAVE_TERMIOS_H) && defined(HAVE_DEV_TTY)
     struct termios tty_attr;

     if ((tty_fd = open(DEV_TTY, O_RDONLY | O_NONBLOCK)) < 0)
          tty_error(ERR_DEV_TTY, "can't open %s", DEV_TTY);

     tcgetattr(tty_fd, &tty_attr);
     tty_saved_attr = tty_attr;

     atexit(tty_done);
#endif

     /* note:  killed from keyboard (^C) or killed [-9]
      *        SIGINT, SIGQUIT, SIGTERM, SIGKILL
      */
     signal(SIGINT, sig_handle_abort);
     signal(SIGTERM, sig_handle_abort);

     atexit(sig_remove_abort);
     atexit(mem_free_all);
}

#if defined(HAVE_TERMIOS_H) && defined(HAVE_DEV_TTY)
void
tty_done(void)
{
     tcsetattr(tty_fd, TCSANOW, &tty_saved_attr);
     close(tty_fd);
}
#endif

extern int
tty_error(u_int msg_code, const char *fmt, ...)
{
     va_list arg_ptr;

     if (!usr_opt.verbose)   /* quiet mode, only return codes */
          exit(msg_code & 0xFF);

     fprintf(stderr, "** ERROR: ");

     va_start(arg_ptr, fmt);
     vfprintf(stderr, fmt, arg_ptr);
     va_end(arg_ptr);

     fprintf(stderr, "\nenter `%s --help' if you need help\n\n", PACKAGE);

     exit(msg_code & 0xFF);
}

extern void
tty_warning(const char *fmt, ...)
{
     va_list arg_ptr;

     if (!usr_opt.verbose)   /* no warning messages in quiet mode */
          return;

     fprintf(stderr, "** WARNING: ");

     va_start(arg_ptr, fmt);
     vfprintf(stderr, fmt, arg_ptr);
     va_end(arg_ptr);

     fprintf(stderr, "\n");
}

extern void
tty_message(const char *fmt, ...)
{
     va_list arg_ptr;

     va_start(arg_ptr, fmt);
     vfprintf(stdout, fmt, arg_ptr);
     va_end(arg_ptr);

     fflush(stdout);
}
