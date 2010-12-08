AM_CPPFLAGS = \
	-I$(top_srcdir)/src/include \
	-I$(top_srcdir)/src/activenode \
	-I$(top_srcdir)/src/core
AM_CXXFLAGS = @GENERAL_CXXFLAGS@
lib_LTLIBRARIES = libactivenode.la
libactivenode_la_SOURCES = \
	activenode.cc \
	activenodeparams.cc \
	activenodeperiodiccheck.cc \
	processslotupdater.cc
bin_PROGRAMS = activenode
activenode_LDADD = \
	libactivenode.la \
        $(top_builddir)/src/core/libcluster.la
activenode_SOURCES = \
	main.cc
activenodedir = $(includedir)/activenode
dist_activenode_HEADERS = \
	activenode.h \
	activenodeparams.h \
	activenodeperiodiccheck.h \
	processslotupdater.h
