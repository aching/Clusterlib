INCLUDES = \
        -I@top_srcdir@/src/include \
        -I@top_srcdir@/src/core \
	-I@LOG4CXX_INCLUDES@ \
	-I@BOOST_INCLUDES@ \
	-I@ZOOKEEPER_INCLUDES@
METASOURCES = AUTO
bin_PROGRAMS = cli
cli_LDADD = \
	$(top_builddir)/src/core/libcluster.a \
	@READLINE_LIB@ \
	@NCURSES_LIB@ \
	@LOG4CXX_LIB@ \
	@ZOOKEEPER_LIB@ \
	@APR_LIB@ \
	@APRUTIL_LIB@ \
	@EXPAT_LIB@ \
	@LIB_THREAD_OS@
cli_SOURCES = \
	cliparams.cc \
	cli.cc
#cli_DEPENDENCIES =

CXXFLAGS = @BASE_CXXFLAGS@

@makefile_inc@
