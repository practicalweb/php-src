dnl $Id: config.m4,v 1.27.2.5 2008/07/14 09:02:26 jani Exp $
dnl config.m4 for extension phar

PHP_ARG_ENABLE(phar, for phar archive support,
[  --disable-phar          Disable phar support], yes)

if test "$PHP_PHAR" != "no"; then
  PHP_NEW_EXTENSION(phar, util.c tar.c zip.c stream.c func_interceptors.c dirstream.c phar.c phar_object.c phar_path_check.c, $ext_shared)
  PHP_ADD_BUILD_DIR($ext_builddir/lib, 1)
  PHP_SUBST(PHAR_SHARED_LIBADD)
  AC_MSG_CHECKING([for phar openssl support])
  if test "$PHP_OPENSSL_SHARED" = "yes"; then
    AC_MSG_RESULT([no (shared openssl)])
  else
    if test "$PHP_OPENSSL" = "yes"; then
      AC_MSG_RESULT([yes])
      AC_DEFINE(PHAR_HAVE_OPENSSL,1,[ ])
    else
      AC_MSG_RESULT([no])
    fi
  fi
  PHP_ADD_EXTENSION_DEP(phar, spl, true)
  PHP_ADD_MAKEFILE_FRAGMENT
fi