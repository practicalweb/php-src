dnl
dnl $Id: config.m4,v 1.6 2001/11/30 18:59:31 sniper Exp $
dnl

AC_ARG_WITH(dbase,[],[enable_dbase=$withval])

PHP_ARG_ENABLE(dbase,whether to enable dbase support,
[  --enable-dbase          Enable the bundled dbase library])

if test "$PHP_DBASE" = "yes"; then
  AC_DEFINE(DBASE,1,[ ])
  PHP_EXTENSION(dbase, $ext_shared)
fi
