#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.67])
AC_INIT([omadmclient], [0.0.1], [david.navarro@intel.com])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([foreign])
LT_PREREQ([2.2])
LT_INIT
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES])

AC_CONFIG_MACRO_DIR([m4])
AM_SILENT_RULES([yes])

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O

AC_ARG_ENABLE([logging], AS_HELP_STRING([--enable-tests], [Build test client and plugins]))
AC_ARG_ENABLE([tests], AS_HELP_STRING([--enable-tests], [Build test client and plugins]))

# Checks for libraries.
PKG_PROG_PKG_CONFIG(0.16)
PKG_CHECK_MODULES([LIBXML], [libxml-2.0])
PKG_CHECK_MODULES([LIBWBXML], [libwbxml2])
PKG_CHECK_MODULES([GNUTLS], [gnutls])
AC_CHECK_LIB(dl, dlopen)
AS_IF([test "x$enable_tests" = "xyes"], [
    PKG_CHECK_MODULES([LIBSOUP], [libsoup-2.4])
    PKG_CHECK_MODULES([SQLITE3], [sqlite3])
])

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

if test "x${logging}" = xyes; then
   AC_DEFINE([DMC_LOGGING], 1, [logging enabled])
fi

# set plugins directory
if test "${prefix}" = "NONE"; then
    prefix=${ac_default_prefix}
fi
if test "${exec_prefix}" = "NONE"; then
    exec_prefix=${prefix}
fi
libdir=${exec_prefix}/lib/${PACKAGE}
mo_install_dir=${exec_prefix}/lib/${PACKAGE}/plugins
AC_SUBST([MO_INSTALL_DIR], [${mo_install_dir}])
AC_DEFINE_UNQUOTED([MO_INSTALL_DIR], ["${mo_install_dir}"], [management objects installation directory])


AM_CONDITIONAL([BUILD_TESTS], [test "x${enable_tests}" = xyes])


AC_CONFIG_FILES([Makefile
                 dmcore/Makefile
		         externals/Makefile
		         externals/SyncMLRTK/Makefile
		         tests/Makefile])

AC_OUTPUT

echo \
"-------------------------------------------------

${PACKAGE_NAME} Version ${PACKAGE_VERSION}

Prefix: '${prefix}'.
Compiler: '${CC} ${CFLAGS} ${CPPFLAGS}'

Package features:
	enable-tests: ${enable_tests}

 --------------------------------------------------"
