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
dnl cc_GCC_SR_BUG -- Check for gcc strength-reduce bug
dnl --------------------------------------------------

AC_DEFUN([cc_GCC_SR_BUG],
[AC_CACHE_CHECK(
   [for gcc strength-reduce bug],
   [ac_cv_gcc_strength_bug],
   [AC_RUN_IFELSE(
      [AC_LANG_SOURCE([[
int L[4] = {0,1,2,3};
int main(void)
{
   static int Array[3];
   unsigned int B = 3;
   int i;
   for(i = 0; i < B; i++) Array[i] = i - 3;
   for(i = 0; i < 4 - 1; i++) L[i] = L[i + 1];
   L[i] = 4;
   exit(Array[1] != -2 || L[2] != 3);
}
      ]])],
      [ac_cv_gcc_strength_bug="no"],
      [ac_cv_gcc_strength_bug="yes"],
      [ac_cv_gcc_strength_bug="yes"])])
])[]dnl @end 1 [cc_GCC_SR_BUG]


dnl @start 2
dnl cc_GCC_MISC_BUG -- Check for gcc multiple-implicit-struct-copy bug
dnl (bug in GCC 3.x, prior to version 3.2.3; at least gcc-3.2 and gcc-3.2.2)
dnl ------------------------------------------------------------------------

AC_DEFUN([cc_GCC_MISC_BUG],
[AC_CACHE_CHECK(
   [for gcc multiple-implicit-struct-copy bug],
   [ac_cv_gcc_struct_cp_bug],
   [AC_RUN_IFELSE(
      [AC_LANG_SOURCE([[
typedef struct {
   int _0, _1, _2;
} POINT;
POINT xform(POINT p) {
   return (POINT) { p._0 + 1, p._1 + 2, p._2 + 3 };
}
/* bugged gcc versions return:
 *   0 1 0 ;  1 0 0 ;  1 2 1 ;  2 3 4
 */
int main(void)
{
   int i;
   POINT p[4] =
    { xform((POINT) { 1, 0, 0 }),
      xform((POINT) { 0, 1, 0 }),
      xform((POINT) { 0, 0, 1 }),
      xform((POINT) { 1, 1, 1 }) };

   exit(p[0]._0 != 2 || p[0]._1 != 2 || p[0]._2 != 3 ||
        p[1]._0 != 1 || p[1]._1 != 3 || p[1]._2 != 3 ||
        p[2]._0 != 1 || p[2]._1 != 2 || p[2]._2 != 4 ||
        p[3]._0 != 2 || p[3]._1 != 3 || p[3]._2 != 4);
}
      ]])],
      [ac_cv_gcc_struct_cp_bug="no"],
      [ac_cv_gcc_struct_cp_bug="yes"],
      [ac_cv_gcc_struct_cp_bug="no"])])
])[]dnl @end 2 [cc_GCC_MISC_BUG]
