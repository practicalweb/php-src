dnl
dnl $Id: config.m4,v 1.16.2.1 2002/03/20 00:40:53 sniper Exp $
dnl

PHP_ARG_WITH(fdftk, for FDF support,
[  --with-fdftk[=DIR]      Include FDF support.])

if test "$PHP_FDFTK" != "no"; then

  for i in /usr /usr/local $PHP_FDFTK; do
    if test -r $i/include/FdfTk.h; then
      FDFTK_DIR=$i
    elif test -r $i/include/fdftk.h; then
      AC_DEFINE(HAVE_FDFTK_H_LOWER,1,[ ])
      FDFTK_DIR=$i
    fi
  done

  if test -z "$FDFTK_DIR"; then
    AC_MSG_ERROR(FdfTk.h or fdftk.h not found. Please reinstall the fdftk distribution.)
  fi

  PHP_ADD_INCLUDE($FDFTK_DIR/include)
  
  FDFLIBRARY=""
  for i in fdftk FdfTk; do
    PHP_CHECK_LIBRARY($i, FDFOpen, [FDFLIBRARY=$i], [], [-L$FDFTK_DIR/lib -lm])
  done
  
  if test -z "$FDFLIBRARY"; then
    AC_MSG_ERROR(fdftk module requires >= fdftk 2.0)
  fi
  
  AC_DEFINE(HAVE_FDFLIB,1,[ ])
  PHP_ADD_LIBRARY_WITH_PATH($FDFLIBRARY, $FDFTK_DIR/lib, FDFTK_SHARED_LIBADD)

  PHP_SUBST(FDFTK_SHARED_LIBADD)
  PHP_EXTENSION(fdf, $ext_shared)
fi
