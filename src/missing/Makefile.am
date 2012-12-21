# This file is part of `C5C'.
# Copyright (C) 2002-2005 by Davide Madrisan <davide.madrisan@gmail.com>
#
# This program is free software; you can redistribute it and/or modify it under
# the terms of version 2 of the GNU General Public License as published by the
# Free Software Foundation.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY, to the extent permitted by law; without even the implied
# warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc.,
# 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.

# config.h is in `$(top_builddir)'
AM_CPPFLAGS = -I$(top_srcdir)/src/include \
              -I$(top_builddir)/src/include \
              -I$(top_builddir)

EXTRA_DIST = atexit.c malloc.c memset.c strtol.c

noinst_LIBRARIES = libc5c.a
libc5c_a_SOURCES = getopt.c getopt1.c

libc5c_a_LIBADD = @LIBOBJS@
libc5c_a_DEPENDENCIES = $(libc5c_a_LIBADD)
