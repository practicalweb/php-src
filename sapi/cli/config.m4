dnl
dnl $Id: config.m4,v 1.3.2.3 2002/04/14 03:52:16 sniper Exp $
dnl

AC_MSG_CHECKING(for CLI build)

AC_ARG_ENABLE(cli,
[  --enable-cli            EXPERIMENTAL: Enable building CLI version of PHP.],
[
  PHP_SAPI_CLI=$enableval
],[
  PHP_SAPI_CLI=no
])

if test "$PHP_SAPI_CLI" != "yes"; then
  PHP_DISABLE_CLI
fi

AC_MSG_RESULT($PHP_SAPI_CLI)
