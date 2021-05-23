if INCLUDE_LIBRF24
RF24_LIB=-lrf24
endif

bin_PROGRAMS = rf24pi

rf24pi_SOURCES = main.cpp loop.cpp loop.hpp

rf24pi_LDADD = $(RF24_LIB)

default_datadir = ${sysconfdir}/default
default_data_DATA = default/${PACKAGE}

init_ddir = ${sysconfdir}/init.d
init_d_SCRIPTS = init.d/${PACKAGE}

DISTCLEANFILE = default/ init.d/