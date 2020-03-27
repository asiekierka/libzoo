#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([libzoo], [0], [kontakt at asie.pl])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_SRCDIR([src/zoo.h])
AC_CONFIG_MACRO_DIRS([m4])
AM_INIT_AUTOMAKE([foreign -Wall subdir-objects])
AM_SILENT_RULES([yes])

# Checks for programs.
AC_PROG_CC
AC_PROG_SED
AM_PROG_AR
LT_INIT

# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE

# Checks for library functions.

# libzoo configuration.
AC_ARG_ENABLE([zoo-file-io],
  [AS_HELP_STRING([--disable-zoo-file-io],
    [Build without file I/O])],
  [enable_zoo_file_io=$enableval],
  [enable_zoo_file_io=yes])
AS_IF([test "$enable_zoo_file_io" = yes],
  [AC_DEFINE([ZOO_CONFIG_FILE_IO], [1],
    [Define to build with filesystem I/O])])

AC_ARG_ENABLE([zoo-editor_constants],
  [AS_HELP_STRING([--enable-zoo-editor-constants],
    [Build with editor constants])],
  [enable_zoo_editor_constants=$enableval],
  [enable_zoo_editor_constants=no])
AS_IF([test "$enable_zoo_editor_constants" = yes],
  [AC_DEFINE([ZOO_CONFIG_EDITOR_CONSTANTS], [1],
    [Define to build with editor constants])])

AC_ARG_ENABLE([zoo-optimizations],
  [AS_HELP_STRING([--disable-zoo-optimizations],
    [Build without ZZT-level optimizations])],
  [enable_zoo_optimizations=$enableval],
  [enable_zoo_optimizations=yes])
AS_IF([test "$enable_zoo_optimizations" = yes],
  [AC_DEFINE([ZOO_CONFIG_OPTIMIZATIONS], [1],
    [Define to build with ZZT-level optimizations])])

AC_CONFIG_HEADERS([zoo_config.h])
AC_CONFIG_FILES([Makefile])

# Remove the ZOO_ constants from config.h, so that we don't
# double-define them. The template in zoo_config.h is maintained
# manually. At the time of writing, autoconf 2.69 is the latest
# release version, but there is an unreleased change which would make
# this unnecessary.
AC_CONFIG_COMMANDS([strip-zoo-constants],
  [$SED -e '/ZOO/d' config.h > config.h.tmp; mv -f config.h.tmp config.h])
AC_OUTPUT
