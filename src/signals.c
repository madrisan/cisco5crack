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

#include <signal.h>
#if HAVE_STDLIB_H
# include <stdlib.h>
#endif

#include "params.h"
#include "session.h"
#include "signals.h"
#include "tty.h"

typedef enum e_bool { false = 0, true } bool;
bool save_session_if_signal_abort = false;

void
sig_handle_abort(int unused_sig_num)
{
     /* just to avoid the compiler warning: unused parameter `signum' */
     unused_sig_num += 0;

     if (save_session_if_signal_abort) {
          session_save();
          tty_message("\nsession aborted! - session saved on `%s'\n",
                      RESTORE_FILE);
     } else
          tty_message("\nsession aborted!\n");

     exit(SIGABORT);
}

void
sig_remove_abort(void)
{
     signal(SIGINT, SIG_DFL);
     signal(SIGTERM, SIG_DFL);
}
