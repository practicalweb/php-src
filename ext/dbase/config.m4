dnl $Id: config.m4,v 1.3 1999/12/30 04:07:21 sas Exp $

AC_MSG_CHECKING(whether to include the bundled dbase library)
AC_ARG_WITH(dbase,
[  --with-dbase            Include the bundled dbase library],
[
  if test "$withval" = "yes"; then
	AC_MSG_RESULT(yes)
	AC_DEFINE(DBASE,1,[ ])
	PHP_EXTENSION(dbase)
  else
	AC_MSG_RESULT(no)
	AC_DEFINE(DBASE,0,[ ])
  fi
],[
  AC_MSG_RESULT(no)
  AC_DEFINE(DBASE,0,[ ])
])
