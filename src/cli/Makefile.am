AM_CPPFLAGS = \
        -I@top_srcdir@/src/include \
        -I@top_srcdir@/src/cli \
        -I@top_srcdir@/src/core 
AM_CXXFLAGS = @GENERAL_CXXFLAGS@
lib_LTLIBRARIES = libcli.la
libcli_la_SOURCES = \
	generalcommands.cc \
	cliparams.cc \
	clicommand.cc
bin_PROGRAMS = cli
cli_LDADD = \
	libcli.la \
	$(top_builddir)/src/core/libcluster.la
cli_SOURCES = \
	cli.cc
clidir = $(includedir)/cli
dist_cli_HEADERS = \
	clicommand.h \
	cliformat.h \
	cliparams.h \
	generalcommands.h
