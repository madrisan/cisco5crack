dnl This file is part of `C5C'.
dnl Copyright (C) 2002-2005 by Davide Madrisan <davide.madrisan@gmail.com>

dnl This file is free software; as a special exception the author gives
dnl unlimited permission to copy and/or distribute it, with or without
dnl modifications, as long as this notice is preserved.
dnl
dnl This program is distributed in the hope that it will be useful, but
dnl WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
dnl implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
dnl See the GNU General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License along
dnl with this program; if not, write to the Free Software Foundation, Inc.,
dnl 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.

dnl Process this file with autoconf to produce a configure script.

dnl @start 1
dnl cc_TRY_CFLAGS_WARNINGS -- Checks for CC warnings flags
dnl ------------------------------------------------------

AC_DEFUN([cc_TRY_CFLAGS_WARNINGS],
[AC_ARG_ENABLE([gcc-warnings],
[AS_HELP_STRING([--enable-gcc-warnings],[turn on most of GCC warnings])],
[case "${enableval}" in
   yes)
      cc_TRY_CFLAGS([-Wall])
      cc_TRY_CFLAGS([-W])
      cc_TRY_CFLAGS([-Wcast-qual])
      cc_TRY_CFLAGS([-Wformat])
      cc_TRY_CFLAGS([-Wmissing-declarations])
      cc_TRY_CFLAGS([-Wmissing-prototypes])
      cc_TRY_CFLAGS([-Wmissing-noreturn])
      cc_TRY_CFLAGS([-Wmissing-format-attribute])
      cc_TRY_CFLAGS([-Wunreachable-code])
      cc_TRY_CFLAGS([-Wshadow])
      cc_TRY_CFLAGS([-Wstrict-prototypes])
      cc_TRY_CFLAGS([-Wunused])
      cc_TRY_CFLAGS([-Wpointer-arith])
      cc_TRY_CFLAGS([-Wsign-compare])
      cc_TRY_CFLAGS([-Wbad-function-cast])
      cc_TRY_CFLAGS([-Wwrite-strings])
      cc_TRY_CFLAGS([-pedantic-errors])
   ;;
   no) ;;
   *) AC_MSG_ERROR([bad value ${enableval} for gcc-warnings option]) ;;
 esac],
   [enableval=no])
])[]dnl @end 1 [cc_TRY_CFLAGS_WARNINGS]
