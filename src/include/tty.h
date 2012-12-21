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

#ifndef _TTY_H
#define _TTY_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif

/* program return codes     mask: 0000.0011 */
#define SUCCESS         0x00   /* 0000.0000 */
#define FAILURE         0x01   /* 0000.0001 */
#define SIGABORT        0x02   /* 0000.0010 */
#define INTERNAL        0x03   /* 0000.0011 */

/* user errors              mask: 0000.1100 */
#define ERR_USAGE       0x04   /* 0000.0100 */
#define ERR_PARSER      0x08   /* 0000.1000 */
#define ERR_INFO_RFILE  0x0C   /* 0000.1100 */

/* system errors            mask: 0011.0000 */
#define ERR_OUTOFMEM    0x10   /* 0001.0000 */
#define ERR_DEV_TTY     0x20   /* 0010.0000 */
#define ERR_IO_FILE     0x30   /* 0011.0000 */
#define ERR_FORKING     0x40   /* 0100.0000 */

/*
 * the `/dev/tty1' device corresponds with your first console.
 * When you read from it, you will read from its keyboard buffer, and when you
 * write to it, you will write to its screen.
 * `/dev/tty2' corresponds to the second console, and so on.
 *
 * `/dev/tty' (and `/dev/tty0') correspond to the current console.
 */
#if defined(HAVE_TERMIOS_H) && defined(HAVE_DEV_TTY)
# define DEV_TTY        "/dev/tty"
#endif

#define FIXME(s)        fprintf(stderr, "FIXME : %s\n", s);

extern void tty_init(void);
#if defined(HAVE_TERMIOS_H) && defined(HAVE_DEV_TTY)
extern void tty_done(void);
#endif
extern int tty_error(u_int msg_code, const char *fmt, ...);
extern void tty_warning(const char *fmt, ...);
extern void tty_message(const char *fmt, ...);

#endif
