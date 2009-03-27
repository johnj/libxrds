dnl
dnl $Id: $
dnl

PHP_ARG_ENABLE(xrds, whether to enable xrds support,
[  --enable-xrds       Enable xrds support])

if test -z "$PHP_DEBUG"; then
  AC_ARG_ENABLE(debug,
  [  --enable-debug          compile with debugging symbols],[
    PHP_DEBUG=$enableval
  ],[
    PHP_DEBUG=no
  ])
fi

if test "$PHP_XRDS" != "no"; then
  
  dnl last path for macports
  XRDS_INCLUDE_PATHS="/usr/local /usr /local /opt /opt/local"
  
  XRDS_HEADER="include/xrds/xrds.h" 
  
  if test "$PHP_XRDS" = "yes"; then
    AC_MSG_CHECKING([for libxrds headers])
    for i in $XRDS_INCLUDE_PATHS ; do
      if test -r $i/$XRDS_HEADER; then
        XRDS_DIR=$i
        AC_MSG_RESULT(found in $i)
      fi
    done
  else
    AC_MSG_CHECKING([for libxrds headers in $PHP_XRDS])
	if test -r $PHP_XRDS/$XRDS_HEADER; then
	  XRDS_DIR=$PHP_XRDS
      AC_MSG_RESULT([found])
	fi
  fi

  if test -z "$XRDS_DIR"; then
    AC_MSG_RESULT([not found])
    AC_MSG_ERROR([Cannot find libxrds headers])
  fi
  
  PHP_ADD_INCLUDE($XRDS_DIR/include/xrds)
  
  LIBNAME=xrds
  LIBSYMBOL=xrdsInitialize

  PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  [
    PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $XRDS_DIR/$PHP_LIBDIR, XRDS_SHARED_LIBADD)
    AC_DEFINE(HAVE_XRDSLIB,1,[ ])
  ],[
    AC_MSG_ERROR([wrong libxrds version or lib not found])
  ],[
    -L$XRDS_DIR/$PHP_LIBDIR -lm
  ])
  
  PHP_SUBST(XRDS_SHARED_LIBADD)
  
  PHP_SETUP_LIBXML(XRDS_SHARED_LIBADD, [
        AC_DEFINE(HAVE_XRDS, 1, [whether xrds exists in the system])
        PHP_NEW_EXTENSION(xrds, xrds.c, $ext_shared)
        PHP_EVAL_LIBLINE($LDFLAGS,XRDS_SHARED_LIBADD)
        PHP_SUBST(XRDS_SHARED_LIBADD)
      ], [
        AC_MSG_ERROR([xml2-config not found. Please check your libxml2 installation.])
      ])
  
  if test "$PHP_DEBUG" = "yes"; then
    CFLAGS="$CFLAGS -Wall"
  fi
fi
