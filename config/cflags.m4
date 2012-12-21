dnl Finding valid warning flags for the C Compiler.           -*-Autoconf-*-
dnl
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
dnl cc_TRY_CFLAGS (CFLAGS, [ACTION-IF-WORKS], [ACTION-IF-FAILS])
dnl ------------------------------------------------------------
dnl Checks if $CC supports a given set of CFLAGS.
dnl If supported, the current CFLAGS is appended to SUPPORTED_CFLAGS
AC_DEFUN([cc_TRY_CFLAGS],
   [AC_MSG_CHECKING([whether compiler accepts $1])
   ac_save_CFLAGS="$CFLAGS"
   CFLAGS="$CFLAGS $1"
   AC_COMPILE_IFELSE(
      [[int x;]],
      [ac_cv_try_cflags_ok=yes
       SUPPORTED_CFLAGS="$SUPPORTED_CFLAGS $1"],
      [ac_cv_try_cflags_ok=no])
   CFLAGS="$ac_save_CFLAGS"
   AC_MSG_RESULT([$ac_cv_try_cflags_ok])
   if test x"$ac_cv_try_cflags_ok" = x"yes"; then
      ifelse([$2],[],[:],[$2])
   else
      ifelse([$3],[],[:],[$3])
   fi
])[]dnl @end 1 [cc_TRY_CFLAGS]


dnl @start 2
dnl cc_TRY_CFLAGS_OPT -- Checks for CC optimization flags
dnl -----------------------------------------------------

dnl -O, -O2      : most unix compilers
dnl +K2          : linux-kcc, solaris-kcc
dnl -fast        : linux-pgcc
dnl -O1          : win32-msvc, win32-msvc.net
dnl -Od -Ob1s    : win32-icc
dnl -Gl+ -O -Oc+ : win32-visage
dnl -ox          : win32-watcom
dnl
AC_DEFUN([cc_TRY_CFLAGS_OPT],
[cc_TRY_CFLAGS(-O2,[],
       cc_TRY_CFLAGS(-O,[],
          cc_TRY_CFLAGS(+K2,[],
             cc_TRY_CFLAGS(-fast,[],
                [AC_MSG_WARN([])
                 AC_MSG_WARN([no code optimization flags found])
                 AC_MSG_WARN([])]))))
 cc_TRY_CFLAGS([-fno-exceptions])
 # loop unrolling when iteration count is known
 cc_TRY_CFLAGS([-funroll-loops])
 cc_TRY_CFLAGS([-finline])
 cc_TRY_CFLAGS([-finline-functions])
#cc_TRY_CFLAGS([-fno-strict-aliasing])
])[]dnl @end 2 [cc_TRY_CFLAGS_OPT]


dnl @start 3
dnl cc_TRY_CFLAGS_CPU -- Checks for CC cpu optimisation flags
dnl ---------------------------------------------------------

dnl note (Athlon)
dnl   host      :  i686-pc-linux-gnu
dnl   modelname :  Mobile AMD Athlon(tm) 4 Processor

AC_DEFUN([cc_TRY_CFLAGS_CPU],
[sarchopt=no
 whicharch=

 # note: `host' to base the decisions on the system where `configure' will run
 AC_MSG_CHECKING([for $CC cpu optimization flags])
 case $host in
 i?86*-*-linux* | i?86*-*-cygwin | i386-*-solaris*| i?86-*  | k?-* | athlon-*)
 if test "$GCC" = "yes"; then
    if $CC -mtune=i386 -S -o /dev/null -xc /dev/null >/dev/null 2>&1;
       then sarchopt="-mtune"
    else
       if $CC -mcpu=i386 -S -o /dev/null -xc /dev/null >/dev/null 2>&1;
       then sarchopt="-mcpu"; fi
    fi

    if test x"$sarchopt" != xno; then
       # special check for k6 and k7 cpu CC support
       if $CC $sarchopt=k6 -S -o /dev/null -xc /dev/null >/dev/null 2>&1;
       then k6cpu="k6"
       else k6cpu="i586"; fi

       if $CC $sarchopt=athlon -S -o /dev/null -xc /dev/null >/dev/null 2>&1;
       then k7cpu="athlon"
       else k7cpu="i686"; fi

       case $host in
       i386-*) # *BSD return this even on a PIII #-))
          whicharch=i386 ;;
       i486-*) # oh dear!
          whicharch=i486 ;;
       i586-*) # 586/K5/5x86/6x86/6x86MX
          whicharch=i586
          if test -f /proc/cpuinfo; then
             modelname=`cat /proc/cpuinfo | grep "model\ name" | sed -e 's/.*: //g'`
             case "$modelname" in
                *K6*) whicharch="$k6cpu" ;;
             esac
          fi
          ;;
       i686-*) # Pentium-Pro/Celeron/Pentium-II/Pentium-III/Pentium-4/Crusoe
          whicharch=i686
          if test -f /proc/cpuinfo; then
             modelname=`cat /proc/cpuinfo | grep "model\ name" | sed -e 's/.*: //g'`
             case "$modelname" in
             *Athlon* | *Duron* | *K7*)
                whicharch="$k7cpu" ;;
             esac
          fi
          ;;
       k6-*)
          whicharch=k6 ;;
       k7-*)
          whicharch=k7 ;;
       athlon-*)
          whicharch=athlon ;;
       esac

       if test x"$whicharch" != x; then
          SUPPORTED_CFLAGS="$SUPPORTED_CFLAGS $sarchopt=$whicharch"
       fi
    fi  # of `test x"$sarchopt" != "xno"'
 fi
 ;;  # of `test "$GCC" = "yes"'

 ia64-*-linux*)
    if test "$GCC" = "yes"; then
       if $CC -mtune=itanium -S -o /dev/null -xc /dev/null >/dev/null 2>&1;
          then sarchopt="-mtune"
       else
          if $CC -mcpu=itanium -S -o /dev/null -xc /dev/null >/dev/null 2>&1;
          then sarchopt="-mcpu"; fi
       fi

       if test x"$sarchopt" != xno; then
          if $CC $sarchopt=itanium2 -S -o /dev/null -xc /dev/null >/dev/null 2>&1;
          then whicharch="itanium2"
          else whicharch="itanium1"; fi
       fi
       if test x"$whicharch" != x; then
          SUPPORTED_CFLAGS="$SUPPORTED_CFLAGS $sarchopt=$whicharch"
       fi
    fi
 ;;

 sparc-*-solaris*)
 #  -mv7
 #     by default (unless specifically configured for the Fujitsu SPARClite),
 #     gcc generates code for the v7 variant of the SPARC architecture.
 #  -mv8
 #     will give you SPARC v8 code. The only difference from v7 code is that the
 #     compiler emits the integer multiply and integer divide instructions which
 #     exist in SPARC v8 but not in SPARC v7.
 #  -msparclite
 #     will give you SPARClite code. This adds the integer multiply, integer
 #     divide step and scan (ffs) instructions which exist in SPARClite but not
 #     in SPARC v7.
 #  -mtune=cpu_type
 #     Set the instruction set, register set, and instruction scheduling
 #     parameters for machine type cpu_type. Supported values for cpu_type are
 #        v7, cypress, v8, supersparc, sparclite, hypersparc, sparclite86x,
 #        f930, f934, sparclet, tsc701, v9, ultrasparc, ultrasparc3.
 #     Default instruction scheduling parameters are used for values that select
 #     an architecture and not an implementation.
 #     These are v7, v8, sparclite, sparclet, v9.
 #
 #     Here is a list of each supported architecture and their supported
 #     implementations.
 #        v7:            cypress
 #        v8:            supersparc, hypersparc
 #        sparclite:     f930, f934, sparclite86x
 #        sparclet:      tsc701
 #        v9:            ultrasparc, ultrasparc3
 #  -m32
 #  -m64
 if test "$GCC" = yes; then
    machine=`(uname -m) 2>/dev/null` || \
       machine=`(uname -p) 2>/dev/null` || machine="unknown"
    case $machine in
    sun4c) whicharch="v7"; mtune_cflags="-mtune=supersparc" ;;
    sun4m) whicharch="v8"; mtune_cflags="-mtune=supersparc" ;;
    sun4u)
       case `$CC --version 2>/dev/null` in
       1.* | 2.*)
       # -mtune=ultrasparc can trigger a GCC 2.95.x compiler bug
       # gcc: Internal compiler error: program cc1 got fatal signal 11
       # avoid -mtune=ultrasparc with gcc 2.*
       whicharch="v8"; mtune_cflags="-mtune=ultrasparc"
       ;;
       *)
       # GCC 3 or newer should have no problem with -mtune=ultrasparc
       whicharch="ultrasparc"; mtune_cflags="-mtune=ultrasparc"
       ;;
       esac
    ;;
    *) whicharch="v7"; mtune_cflags= ;;
    esac
    sarchopt="-mtune"
    SUPPORTED_CFLAGS="$SUPPORTED_CFLAGS -mcpu=$whicharch $mtune_cflags"
 fi
 ;;

 # *-*-msdos* | *-*-windows*) ;;
 esac

if test x"$whicharch" = x -a x"$cpu_cflags" = x ; then
   AC_MSG_RESULT([none found])
   AC_MSG_WARN([])
   AC_MSG_WARN([Sorry. No cpu optimization support.])
   AC_MSG_WARN([You should manually set the CFLAGS variable to enable it.])
   AC_MSG_WARN([])
else
   AC_MSG_RESULT([$sarchopt=$whicharch])
fi
])[]dnl @end 3 [cc_TRY_CFLAGS_CPU]


dnl @start 4
dnl cc_TRY_CFLAGS_SSP -- Checks for GCC stack-smashing protector
dnl ------------------------------------------------------------
dnl See <http://researchweb.watson.ibm.com/trl/projects/security/ssp/>
dnl for more informations

AC_DEFUN([cc_TRY_CFLAGS_SSP],
[cc_TRY_CFLAGS(-fstack-protector,[])])[]dnl @end 4 [cc_TRY_CFLAGS_SSP]
