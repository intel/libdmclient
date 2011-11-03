#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.67])
AC_INIT([dmclient], [0.0.1], [david.navarro@intel.com])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([color-tests])
LT_PREREQ([2.2])
LT_INIT
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES])

AC_CONFIG_MACRO_DIR([m4])
# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_LIBTOOL

# Checks for libraries.
PKG_PROG_PKG_CONFIG(0.16)
PKG_CHECK_MODULES([SQLITE3], [sqlite3])
PKG_CHECK_MODULES([LIBXML], [libxml-2.0])
PKG_CHECK_MODULES([LIBWBXML], [libwbxml2])
PKG_CHECK_MODULES([GNUTLS], [gnutls])
PKG_CHECK_MODULES([LIBSOUP], [libsoup-2.4])

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([memset])

if test "x${CFLAGS}" = "x"; then
   CFLAGS="-O2"
fi

AM_SILENT_RULES([yes])

AC_ARG_ENABLE([logging], [  --enable-logging enables logging], [logging=${enableval}], [logging=no])

if test "x${logging}" = xyes; then
   AC_DEFINE([DMCOMMON_LOGGING], 1, [logging enabled])
fi

AC_CONFIG_FILES([Makefile
                 dmcore/Makefile
		 externals/Makefile
		 externals/SyncMLRTK/Makefile
		 tests/Makefile
		 tests/client/Makefile])
AC_OUTPUT

echo \
"-------------------------------------------------

${PACKAGE_NAME} Version ${PACKAGE_VERSION}

Prefix: '${prefix}'.
Compiler: '${CC} ${CFLAGS} ${CPPFLAGS}'

Package features:
	enable-logging: ${logging}

 --------------------------------------------------"
