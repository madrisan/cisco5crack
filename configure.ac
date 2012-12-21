# This file is part of `C5C'.
# Copyright (C) 2002-2005 Davide Madrisan <davide.madrisan@gmail.com>

# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY, to the extent permitted by law; without even the implied
# warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along  with
# this program; if not, write to the Free Software Foundation, Inc.,
# 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA


# Process this file with autoconf to produce a configure script.

## Initialization
## --------------
m4_define(PROG_VERSION,regexp(m4_include(VERSION),[version \([.0-9A-Za-z]+\)],[\1]))
m4_define(PROG_NAME, [c5c])
m4_define(PROG_BUGREPORT, [davide.madrisan@gmail.com])

# Group used for security purposes while saving program output
m4_define(SYSGROUP, [PROG_NAME])

AC_PREREQ(2.61)
AC_INIT([PROG_NAME],[PROG_VERSION],[PROG_BUGREPORT],[PROG_NAME])

# Use `config.h' instead of -D macros (@DEFS@ will be `-DHAVE_CONFIG_H')
# note: AC_CONFIG_HEADERS(src/include/config.h:src/include/config.hin)
#   seems not to be compatible with the GNU gettext i18n stuff
AC_CONFIG_HEADERS(config.h:config.hin)
AC_CONFIG_SRCDIR(src/decrypt.c)
# set `ac_aux_dir' = 'config'
AC_CONFIG_AUX_DIR(config)

# Just to display what's the version number
AC_MSG_CHECKING([for PROG_NAME version])
AC_MSG_RESULT([PROG_VERSION])

# Compute the canonical build-system type variable. `build' and its three
# individual parts `build_cpu', `build_vendor' , `build_os'
#AC_CANONICAL_HOST
AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE([1.10 check-news dist-bzip2 nostdinc])

AC_COPYRIGHT([
This file is part of PROG_NAME.
Copyright (C) 2002-2004 Davide Madrisan <PROG_BUGREPORT>
])
AC_REVISION($Revision: 1.02 $)


## Checks for programs
## -------------------
# AC_PROG_INSTALL
# dnl Prepend src dir to install path dir if it's a relative path
# case "$INSTALL" in
#   [[\\/$]]* | ?:[[\\/]]* ) ;;
#   *)  INSTALL="\\\$(TOPSRCDIR)/$INSTALL" ;;
# esac

# AC_DISABLE_SHARED
# AC_PROG_LIBTOOL

#MYPATH="/bin:/usr/bin:/usr/local/bin:$PATH:/sbin:/usr/sbin:/usr/local/sbin:/opt/gnu/bin"
MYPATH=$PATH

AC_CHECK_TOOL(LD,ld,ld,$MYPATH)
AC_CHECK_TOOL(AR,ar,ar,$MYPATH)
AC_PROG_RANLIB
AC_CHECK_TOOL(STRIP,strip,strip,$MYPATH)

# Define the variable `SET_MAKE' if `MAKE' is empty and calls `AC_SUBST' for `SET_MAKE'
#AC_PROG_MAKE_SET
# Check to see if function prototypes are understood by the compiler.
# This test must precede tests of compiler characteristics like that for the inline
# keyword, since it may change the degree to which the compiler supports such features.
AM_C_PROTOTYPES

# Determine a C compiler to use. Define `CFLAGS', `EXEEXT', `OBJEXT'
# Define `CFLAGS' to `-g -O2' for the GNU C compiler. or `-g' for other compilers.
# If the C compiler is not in ANSI C mode by default, try to add an option to output
# variable `CC' to make it so.
AC_PROG_CC
# AC_GNU_SOURCE
if test "x$ac_cv_prog_cc_stdc" = xno; then
   AC_MSG_ERROR([$CC is not an ANSI C compiler.])
fi
AC_PROG_GCC_TRADITIONAL

#AC_PROG_INSTALL
#AC_PROG_LN_S

# Set the default `prefix' variable to `/usr'
#AC_PREFIX_DEFAULT([/usr])

INCLUDEPATH=

AC_LANG([C])
#AM_GNU_GETTEXT
#ALL_LINGUAS="it"

SUPPORTED_CFLAGS=
# Add CC warnings flags to `SUPPORTED_CFLAGS'
cc_TRY_CFLAGS_WARNINGS
#
AC_ARG_ENABLE([gcc-ansi],
AS_HELP_STRING([--disable-gcc-ansi],
               [don't run the compiler in strict ansi mode]),
   [gcc_ansi_mode=$enableval], [gcc_ansi_mode=yes])
if test "x$gcc_ansi_mode" = xyes; then
   cc_TRY_CFLAGS([-ansi])
fi
#
AC_ARG_ENABLE([gcc-profile],
AS_HELP_STRING([--enable-gcc-profile],
               [enable profiling (development only)]),
   [gcc_profile_mode=$enableval], [gcc_profile_mode=no])
if test "x$gcc_profile_mode" = xyes; then
   cc_TRY_CFLAGS([-pg])
fi

# Add CC optimization flags to `SUPPORTED_CFLAGS'
cc_TRY_CFLAGS_OPT
# note: -pg and -fomit-frame-pointer are incompatible
if test "x$gcc_profile_mode" = xno; then
   cc_TRY_CFLAGS([-fomit-frame-pointer])
fi

# Add CC cpu optimisation flags to `SUPPORTED_CFLAGS'
cc_TRY_CFLAGS_CPU

if test "x$GCC" = "xyes"; then
#  Check for gcc strength-reduce bug
   cc_GCC_SR_BUG
   if test "x$ac_cv_gcc_strength_bug" = xyes; then
      SUPPORTED_CFLAGS="$SUPPORTED_CFLAGS -fno-strength-reduce"
   fi
# Check for gcc multiple-implicit-struct-copy bug
   cc_GCC_MISC_BUG
fi

# Add GCC stack-smashing protector support
if test "x$GCC" = "xyes"; then
   cc_TRY_CFLAGS_SSP
fi

# Allow user to specify flags
# AC_ARG_WITH([cflags],
# AS_HELP_STRING([--with-cflags],
#                [specify additional flags to pass to compiler]),
#   [if test x$withval != xno; then CFLAGS="$CFLAGS $withval"; fi])

AC_SUBST([CFLAGS], ["$SUPPORTED_CFLAGS $CFLAGS"])
AC_CACHE_SAVE

# Define `WORDS_BIGENDIAN' if a big-endian architecture is found
# AC_C_BIGENDIAN ([ACTION-IF-TRUE], [ACTION-IF-FALSE], [ACTION-IF-UNKNOWN])
# The default for ACTION-IF-TRUE is to define `WORDS_BIGENDIAN'.
# The default for ACTION-IF-FALSE is to do nothing.
# And finally, the default for ACTION-IF-UNKNOWN is to abort configure and tell
# the installer which variable he should preset to bypass this test.
#
AC_C_BIGENDIAN


## Checks for header files
## -----------------------
# Define `STDC_HEADERS' if the system has ANSI C header files
# AC_HEADER_STDC
AC_CHECK_HEADERS([inttypes.h \
                  assert.h \
                  errno.h \
                  fcntl.h \
                  libintl.h \
                  signal.h \
                  stdarg.h \
                  sys/socket.h \
                  sys/un.h],)
AC_HEADER_STAT
AC_HEADER_TIME

# Checks if POSIX termios headers and functions are available.
AC_SYS_POSIX_TERMIOS
if test "x$ac_cv_sys_posix_termios" = xno; then
   AC_MSG_WARN([])
   AC_MSG_WARN([Your system appears not to have installed a proper set of])
   AC_MSG_WARN([POSIX termios headers and functions.])
   AC_MSG_WARN([The program will be built with no interactive terminal support.])
   AC_MSG_WARN([])
else
   AC_DEFINE(HAVE_TERMIOS_H, 1,
      [Define to 1 to if you have the POSIX termios headers and functions.])
fi
AC_CACHE_SAVE


## Checks for library functions for portability issues
## ---------------------------------------------------
# AC_CHECK_DECLS([getopt])
AC_CHECK_FUNCS([getpid memcpy socket strchr umask])
AC_REPLACE_FUNCS([atexit memset strtol])
# If a working `fork' is found, define `HAVE_WORKING_FORK'
# If `vfork.h' is found, define `HAVE_VFORK_H'.
# If a working `vfork' is found, define `HAVE_WORKING_VFORK'.
AC_FUNC_FORK
# If the `malloc' works correctly (`malloc (0)' returns a valid pointer),
# define `HAVE_MALLOC'
AC_FUNC_MALLOC

# Check for -lsocket (needed by Solaris and other OSs)
AC_CHECK_FUNCS(socket,,AC_CHECK_LIB([socket], [socket]))

# Check for libxml2
# note : /usr/bin/xml2-config {--cflags | --libs}
#        xml2-config --cflags
#         -I/usr/include/libxml2 -I/usr/include/libxml2/libxml
#        xml2-config --libs
#         -L/usr/lib -lxml2 -lz -lm -L/usr/X11R6/lib
# AC_CHECK_TOOL(XML2_CONFIG,xml2-config,no)
# if test x"$XML2_CONFIG" = xno; then
#    AC_MSG_ERROR([Could not find xml2-config anywhere,]
#    [check ftp://xmlsoft.org/ for libxml 2.4.x.])
# fi
# AC_CHECK_LIB(xml2, xmlParseFile,
#    [LIBS="$LIBS `xml2-config --libs`"
#     AC_DEFINE(HAVE_LIBXML2,1,
#               [Define to 1 if you have the `xml2' library (-lxml2).])],
#    [AC_MSG_ERROR([Could not find libxml2 anywhere,]
#    [check ftp://xmlsoft.org/ for libxml 2.4.x.])])
# INCLUDEPATH="$INCLUDEPATH `xml2-config --cflags`"

# If `signal.h' declares `signal' as returning a pointer to a function
# returning `void', define `RETSIGTYPE' to be `void';
# otherwise, define it to be `int'.
AC_TYPE_SIGNAL


## Checks for typedefs, structures, and compiler characteristics
## -------------------------------------------------------------
# Check to see if function prototypes are understood by the compiler.
AC_C_PROTOTYPES
# Check if C the compiler does not fully support the ANSI C qualifier `const'.
AC_C_CONST
#AC_C_INLINE
AC_CHECK_TYPE(u_char, [unsigned char])
AC_CHECK_TYPE(u_int, [unsigned int])
AC_CHECK_TYPE(u_long, [unsigned long])
AC_CHECK_TYPE(u_int32_t, [unsigned int])
# case $host in
#    alpha*) AC_CHECK_TYPE(u_int32_t, [unsigned int]) ;;
#    *) AC_CHECK_TYPE(u_int32_t, [unsigned long]) ;;
# esac
AC_CHECK_TYPE(int_fast16_t, [int])
AC_CHECK_TYPE(uint_fast8_t, [unsigned char])
AC_CHECK_TYPE(uint_fast16_t, [unsigned int])
AC_CHECK_TYPE(size_t, unsigned int)

#AC_CHECK_SIZEOF([u_int32_t])

AC_CHECK_MEMBERS([struct sockaddr_un.sun_path], , ,
    [ #include <sys/types.h>
      #include <sys/un.h>])
AC_CACHE_SAVE


## Checks for system services
## --------------------------
# Checks for `/dev/tty' device
if test "x$ac_cv_sys_posix_termios" = xyes; then
   cc_HAVE_DEV_TTY
fi

# Checks for `/dev/null' device
cc_HAVE_DEV_NULL

# Checks for `/dev/random' device
AC_MSG_CHECKING([whether use of /dev/random is requested])
AC_ARG_ENABLE([dev-random-support],
   AS_HELP_STRING([--disable-dev-random],
                  [disable the use of dev random]),
   [dev_random_support=$enableval], [dev_random_support=yes])
AC_MSG_RESULT($dev_random_support)
if test "x$dev_random_support" = xyes; then
   cc_HAVE_DEV_RANDOM
fi

# Checks for `egd' support
AC_MSG_CHECKING([whether the interface to egd is requested])
AC_ARG_ENABLE([egd-support],
AS_HELP_STRING([--disable-egd-support],
               [disable the support for the EGD socket]),
   [egd_support=$enableval], [egd_support=yes])
#
AC_MSG_RESULT($egd_support)
if test "x$egd_support" = xyes; then
   AC_DEFINE(EGD_SUPPORT, 1,
             [Define to 1 to enable the support for the EGD socket.])
fi

# Checks for the compile options `shortpasswd'.
AC_ARG_ENABLE([shortpasswd],
AS_HELP_STRING([--disable-shortpasswd],
               [use the default md5 algorithm (no optimizations!)]),
   [], [enable_shortpasswd=yes])  # default is `enable-shortpasswd'
#
if test "x$enable_shortpasswd" = xyes; then
   AC_DEFINE(SHORT_PASSWORDS, 1,
             [Define to 1 to optimize the md5 algorithm.])
fi

AC_SUBST(ac_aux_dir)
AC_SUBST(INCLUDEPATH)

AC_SUBST(SYSGROUP)
AC_DEFINE(LOGGING_GROUP, SYSGROUP,
          [Group used for security purposes while saving program output.])

# Create Configuration Files.
AC_CONFIG_FILES([Makefile]
                [config/Makefile]
                [dictionary/Makefile]
                [docs/Makefile]
                [src/Makefile]
                [src/include/Makefile]
                [src/missing/Makefile])

# Generate `config.status' and launch it.
# AC_OUTPUT([],
#           [echo "manifest.sh: creating FILES.md5sum"
#            ./config/manifest.sh > FILES.md5sum])

AC_OUTPUT


if test "x$enable_shortpasswd" = xno; then
   cat <<EOF

WARNING:
WARNING: the option '--disable-shortpasswd' has been selected
WARNING: _all_ the optimizations in the MD5 algorithm will be disabled!
WARNING:
EOF
fi

if test "x$ac_cv_gcc_struct_cp_bug" = xyes; then
   cat <<EOF

WARNING:
WARNING: gcc multiple-implicit-struct-copy bug found!
WARNING: this bug is potentially a security risk, because data
WARNING: is unintentionally \"overlapped\" between subsequent
WARNING: struct copies. A carefully crafted exploit may be able
WARNING: to obtain sensitive information, or run arbitrary code
WARNING: (in the case where a struct contains a function pointer)
WARNING:
WARNING: You should upgrade gcc to a version >= 3.2.3
WARNING:
EOF
fi

#cat <<_EOF
#
#Good - your configure finished. Start make now.
#
#_EOF
