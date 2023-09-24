#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.71])
AC_INIT([blazescript], [1.0.0-beta1], [rakinar2@onesoftnet.eu.org])
AC_CONFIG_SRCDIR([src/ast.h])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR([build-aux])

AM_INIT_AUTOMAKE([-Wall])
AM_SILENT_RULES([yes])

# Checks for programs.
AC_PROG_CC
AM_PROG_AS
AC_PROG_RANLIB
AM_PROG_AR

# Checks for libraries.
AC_CHECK_LIB([m], [ceill])

# Checks for header files.
AC_CHECK_HEADERS([inttypes.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_C_RESTRICT
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([atexit strchr strdup strerror strstr])

AC_ARG_ENABLE(
    [debug],
    [  --enable-debug    Turn on debugging],
    [case "${enableval}" in
      yes)
       DEBUG=1
       ;;
      no)  debug=false ;;
      *) AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
    esac],
    [debug=false]
)

AM_CONDITIONAL([DEBUG], [test x"${DEBUG}" != x])

if test "x$build" != "x$host"; then
    BLAZE_TARGET_ARCH="$host"
else
    BLAZE_TARGET_ARCH="$build"
fi

AM_CONDITIONAL([LINUX], [test x"$(echo "$BLAZE_TARGET_ARCH" | grep linux)" != x])
AM_CONDITIONAL([DARWIN], [test x"$(echo "$BLAZE_TARGET_ARCH" | grep darwin)" != x])
AM_CONDITIONAL([WINDOWS], [test x"$(echo "$BLAZE_TARGET_ARCH" | grep win32)" != x || test x"$(echo "$BLAZE_TARGET_ARCH" | grep mingw)" != x])

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 lib/Makefile
                 include/Makefile])
AC_OUTPUT
