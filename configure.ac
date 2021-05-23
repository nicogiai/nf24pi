AC_INIT([rf24pi], [1.0])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_CONFIG_HEADERS([config.h])

AC_PROG_CXX

AC_CHECK_HEADERS([RF24/RF24.h],[RF24_LIB=-lrf24],[RF24_LIB=])
AC_SUBST([RF24_LIB])

have_pthreads=no
AC_SEARCH_LIBS([pthread_create], [pthread], [have_pthreads=yes])

AC_CONFIG_FILES([
 Makefile
 src/Makefile
])
AC_CONFIG_FILES(src/default/${PACKAGE}:src/default.in:)
AC_CONFIG_FILES(src/init.d/${PACKAGE}:src/init.d.in)
AC_OUTPUT