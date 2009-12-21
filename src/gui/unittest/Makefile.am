INCLUDES = \
	-I. \
	-I$(top_srcdir)/src/gui/src \
	-I@LOG4CXX_INCLUDES@ \
	-I@BOOST_INCLUDES@ \
	-I$(top_srcdir)/src/include \
	-I@ZOOKEEPER_INCLUDES@
METASOURCES = AUTO
bin_PROGRAMS = zkuitest
zkuitest_LDADD = \
	$(top_builddir)/src/core/libcluster.a \
	@BOOST_LIB@ \
	@ZOOKEEPER_LIB@ \
	@LOG4CXX_LIB@ \
	@APR_LIB@ \
	@APRUTIL_LIB@ \
	@EXPAT_LIB@ \
	@LIB_THREAD_OS@
zkuitest_SOURCES = \
	test.cc \
	jsontest.cc \
	jsonrpctest.cc \
	util.cc
CXXFLAGS = @BASE_CXXFLAGS@

@makefile_inc@
