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
dnl _cc_HAVE_DEV(tty | random, HAVE_DEV_TTY | DEV_RANDOM_SUPPORT)
dnl ------------------------------------------------------------

AC_DEFUN([_cc_HAVE_DEV],
[AC_CACHE_CHECK([if /dev/$1 exists],
   [cc_cv_sys_dev$1],
   [AC_RUN_IFELSE(
      [AC_LANG_SOURCE([[
#include <fcntl.h>   /* open, O_RDONLY, O_NONBLOCK */
#include <stdlib.h>  /* exit */
#include <unistd.h>  /* close */

int main(int argc, char **argv) {
   int fd = 0;
   if ((fd = open("/dev/$1", O_RDONLY | O_NONBLOCK)) < 0)
      exit(1);
   close(fd);
   exit(0);
}
      ]])],
      [cc_cv_sys_dev$1="yes"],
      [cc_cv_sys_dev$1="no"],
      [cc_cv_sys_dev$1="yes"])])
if test x"$cc_cv_sys_dev$1" = xyes; then
  AC_DEFINE([$2], 1, [Define if /dev/$1 exists.])
fi[]dnl
])[]dnl@end 1 [_cc_HAVE_DEV_TTY]


dnl @start 2
dnl cc_HAVE_DEV_TTY -- Check if /dev/tty exists
dnl -------------------------------------------

AC_DEFUN([cc_HAVE_DEV_TTY], [_cc_HAVE_DEV(tty, HAVE_DEV_TTY)])
dnl @end 2 [cc_HAVE_DEV_TTY]


dnl @start 3
dnl cc_HAVE_DEV_RANDOM -- Check if /dev/random exists
dnl -------------------------------------------------

AC_DEFUN([cc_HAVE_DEV_RANDOM], [_cc_HAVE_DEV(random, DEV_RANDOM_SUPPORT)])
dnl @end 3 [cc_HAVE_DEV_RANDOM]


dnl @start 4
dnl cc_HAVE_DEV_NULL -- Check if /dev/null exists
dnl -------------------------------------------------

AC_DEFUN([cc_HAVE_DEV_NULL], [_cc_HAVE_DEV(null, HAVE_DEV_NULL)])
dnl @end 4 [cc_HAVE_DEV_NULL]
