AC_INIT([simple-tiles], [0.2.0], [https://github.com/propublica/simple-tiles/issues])
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

AC_PROG_CC
AC_PROG_CC_C99
AM_PROG_AR
AC_PROG_LN_S
LT_INIT

PKG_PROG_PKG_CONFIG
PKG_CHECK_MODULES([cairo], [cairo >= 1.12])
PKG_CHECK_MODULES([pango], [pango >= 1.30])
PKG_CHECK_MODULES([pangocairo], [pangocairo])

AX_LIB_GDAL(1.9)

AC_CHECK_HEADERS([assert.h])
AC_CHECK_HEADERS([cairo.h])
AC_CHECK_HEADERS([cpl_error.h])
AC_CHECK_HEADERS([math.h])
AC_CHECK_HEADERS([ogr_api.h])
AC_CHECK_HEADERS([ogr_srs_api.h])
AC_CHECK_HEADERS([pango/pangocairo.h])
AC_CHECK_HEADERS([pthread.h])
AC_CHECK_HEADERS([stdarg.h])
AC_CHECK_HEADERS([stdio.h])
AC_CHECK_HEADERS([stdlib.h])
AC_CHECK_HEADERS([string.h])
AC_CHECK_HEADERS([time.h])

AC_CHECK_FUNCS([atexit])
AC_CHECK_FUNCS([floor])
AC_CHECK_FUNCS([memset])
AC_CHECK_FUNCS([pow])
AC_CHECK_FUNCS([sqrt])
AC_CHECK_FUNCS([strdup])
AC_FUNC_MALLOC
AC_FUNC_STRTOD

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
 Makefile
 src/Makefile
 test/Makefile
 src/simple-tiles.pc
])
AC_OUTPUT
