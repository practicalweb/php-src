dnl
dnl $Id: config.m4,v 1.2 2008/03/19 22:43:24 gwang Exp $
dnl

AC_MSG_CHECKING(for LiteSpeed support)

AC_ARG_WITH(litespeed,
[  --with-litespeed        Build PHP as litespeed module],
[
  PHP_SAPI_LITESPEED=$withval
],[
  PHP_SAPI_LITESPEED=no
])

if test "$PHP_SAPI_LITESPEED" != "no"; then
  PHP_ADD_MAKEFILE_FRAGMENT($abs_srcdir/sapi/litespeed/Makefile.frag,$abs_srcdir/sapi/litespeed,sapi/litespeed)
  SAPI_LITESPEED_PATH=sapi/litespeed/php
  PHP_SUBST(SAPI_LITESPEED_PATH)
  PHP_SELECT_SAPI(litespeed, program, lsapi_main.c lsapilib.c, "", '$(SAPI_LITESPEED_PATH)') 
  case $host_alias in
  *darwin*)
    BUILD_LITESPEED="\$(CC) \$(CFLAGS_CLEAN) \$(EXTRA_CFLAGS) \$(EXTRA_LDFLAGS_PROGRAM) \$(LDFLAGS) \$(NATIVE_RPATHS) \$(PHP_GLOBAL_OBJS:.lo=.o) \$(PHP_SAPI_OBJS:.lo=.o) \$(PHP_FRAMEWORKS) \$(EXTRA_LIBS) \$(ZEND_EXTRA_LIBS) -o \$(SAPI_LITESPEED_PATH)"
    ;;
  *cygwin*)
    SAPI_LITESPEED_PATH=sapi/litespeed/php.exe
    BUILD_LITESPEED="\$(LIBTOOL) --mode=link \$(CC) -export-dynamic \$(CFLAGS_CLEAN) \$(EXTRA_CFLAGS) \$(EXTRA_LDFLAGS_PROGRAM) \$(LDFLAGS) \$(PHP_RPATHS) \$(PHP_GLOBAL_OBJS) \$(PHP_SAPI_OBJS) \$(EXTRA_LIBS) \$(ZEND_EXTRA_LIBS) -o \$(SAPI_LITESPEED_PATH)"
    ;;
  *)
    BUILD_LITESPEED="\$(LIBTOOL) --mode=link \$(CC) -export-dynamic \$(CFLAGS_CLEAN) \$(EXTRA_CFLAGS) \$(EXTRA_LDFLAGS_PROGRAM) \$(LDFLAGS) \$(PHP_RPATHS) \$(PHP_GLOBAL_OBJS) \$(PHP_SAPI_OBJS) \$(EXTRA_LIBS) \$(ZEND_EXTRA_LIBS) -o \$(SAPI_LITESPEED_PATH)"
    ;;
  esac

  PHP_SUBST(BUILD_LITESPEED)
fi

AC_MSG_RESULT($PHP_SAPI_LITESPEED)