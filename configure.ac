#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.71])
AC_INIT([blazescript], [1.0.0-beta1], [rakinar2@onesoftnet.eu.org])
AC_CONFIG_SRCDIR([src/ast.h])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])

AM_INIT_AUTOMAKE([-Wall])
AM_SILENT_RULES([yes])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
# FIXME: Replace 'main' with a function in '-lm':
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
       AC_SUBST([BLAZE_CFLAGS], ["-fsanitize=address -fsanitize=leak"])
       ;;
      no)  debug=false ;;
      *) AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
    esac],
    [debug=false]
)

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT
