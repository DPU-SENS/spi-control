#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
# TO DO: Find the correct version of autoconfig to get this working (octave does not compile the SPI code when this is processed by autoconfig on my mac)

AC_PREREQ([2.67])
AC_INIT([octave instrument control package], [0.0.0+])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_LANG(C++)

# Define macros needed
#AC_DEFINE(__STDC_CONSTANT_MACROS, [], [workaround for C++ programs to use C99 macros])

AC_CHECK_PROG([HAVE_MKOCTFILE], [mkoctfile], [yes], [no])
if [test $HAVE_MKOCTFILE = "no"]; then
  AC_MSG_ERROR([mkoctfile required to install $PACKAGE_NAME])
fi

AC_DEFINE([BUILD_SPI], [], [build SPI interface functions])

# all done

#AC_CONFIG_FILES([Makefile spi/Makefile])
AC_OUTPUT

AC_MSG_NOTICE([

$PACKAGE_NAME is now configured with

   BUILD SPI:         $build_spi

])
