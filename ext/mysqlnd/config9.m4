dnl
dnl $Id: config9.m4,v 1.10 2009/03/17 10:04:57 johannes Exp $
dnl config.m4 for mysqlnd driver


PHP_ARG_ENABLE(mysqlnd_threading, whether to enable threaded fetch in mysqlnd,
[  --enable-mysqlnd-threading
                            MYSQLND: Enable threaded fetch. Note: This forces ZTS on!], no, no)

dnl If some extension uses mysqlnd it will get compiled in PHP core
if test "$PHP_MYSQLND_ENABLED" = "yes"; then
  mysqlnd_sources="mysqlnd.c mysqlnd_charset.c mysqlnd_wireprotocol.c \
                   mysqlnd_ps.c mysqlnd_loaddata.c mysqlnd_palloc.c \
                   mysqlnd_ps_codec.c mysqlnd_statistics.c mysqlnd_qcache.c\
				   mysqlnd_result.c mysqlnd_result_meta.c mysqlnd_debug.c\
				   mysqlnd_block_alloc.c php_mysqlnd.c"

  PHP_NEW_EXTENSION(mysqlnd, $mysqlnd_sources, no)
  PHP_ADD_BUILD_DIR([ext/mysqlnd], 1)
  PHP_INSTALL_HEADERS([ext/mysqlnd])
  PHP_INSTALL_HEADERS([$ext_builddir/php_mysqlnd_config.h])

  dnl Windows uses config.w32 thus this code is safe for now
  if test "$PHP_MYSQLND_THREADING" = "yes"; then
    PHP_BUILD_THREAD_SAFE
    AC_DEFINE([MYSQLND_THREADED], 1, [EXPERIMENTAL: Use mysqlnd internal threading])
  fi

  dnl This creates a file so it has to be after above macros
  PHP_CHECK_TYPES([int8 uint8 int16 uint16 int32 uint32 uchar ulong int8_t uint8_t int16_t uint16_t int32_t uint32_t int64_t uint64_t], [
    $ext_builddir/php_mysqlnd_config.h
  ],[
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif
  ])
fi
