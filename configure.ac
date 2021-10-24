AC_INIT([rf24pi], [1.0])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_CONFIG_HEADERS([config.h])

AC_PROG_CXX

#AC_CHECK_HEADERS([spdlog/spdlog.h])

AC_ARG_WITH(rf24, AC_HELP_STRING(
	[--with-rf24],
	[include deprectated features that are on the chopping block]
	),
	[ 
	rf24="yes"
	AC_DEFINE([HAVE_LIBRF24], [1], ["Define 1 if you have librf24"])
	],
	[rf24="no"]
)
AM_CONDITIONAL(INCLUDE_LIBRF24, test "$rf24" = 'yes' )
                         
AC_SEARCH_LIBS([pthread_create], [pthread])
AC_CHECK_LIB([mosquitto],[main])
AC_CHECK_LIB([curl],[main])

AC_CONFIG_FILES([
 Makefile
 src/Makefile
])
AC_CONFIG_FILES(src/default/${PACKAGE}:src/default.in:)
AC_CONFIG_FILES(src/init.d/${PACKAGE}:src/init.d.in)
AC_OUTPUT