#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
#

AC_PREREQ(2.60)
AC_INIT([trimpack], [0.1], [Mark J. Blair <nf6x@nf6x.net>], [hd])
AC_CONFIG_SRCDIR([trimdump.c])
AC_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE([dist-bzip2 no-dist-gzip])
AM_SILENT_RULES([yes])

AC_SUBST(CFLAGS, [-O2 -Wall])

# Checks for programs.
AC_PROG_CC
AC_PROG_LEX

# Checks for libraries.
AC_CHECK_LIB([m], [sin])

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

# 
AC_CONFIG_FILES([Makefile])

AC_OUTPUT
