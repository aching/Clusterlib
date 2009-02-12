
METASOURCES = AUTO
bin_PROGRAMS = example example2
example_LDADD = \
	$(top_builddir)/common/libcluster.a \
	@LOG4CXX_LIB@ \
	@ZOOKEEPER_LIB@ \
	@APR_LIB@ \
	@APRUTIL_LIB@ \
	@EXPAT_LIB@ \
	@LIB_THREAD_OS@
example2_LDADD = \
	$(top_builddir)/common/libcluster.a \
	@LOG4CXX_LIB@ \
	@ZOOKEEPER_LIB@ \
	@APR_LIB@ \
	@APRUTIL_LIB@ \
	@EXPAT_LIB@ \
	@LIB_THREAD_OS@
CXXFLAGS = -Wall -g @CXXFLAGS@
INCLUDES = \
	-I$(top_srcdir)/common \
	-I@LOG4CXX_INCLUDES@ \
	-I@ZOOKEEPER_INCLUDES@ \
	-I@BOOST_INCLUDES@

example_SOURCES = \
	example.cc
example2_SOURCES = \
	example2.cc

@makefile_inc@
