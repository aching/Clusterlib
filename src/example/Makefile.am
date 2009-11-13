INCLUDES = \
	-I$(top_srcdir)/src/include \
	-I@BOOST_INCLUDES@
METASOURCES = AUTO
bin_PROGRAMS = example example2
example_LDADD = \
	$(top_builddir)/src/core/libcluster.a \
	@LOG4CXX_LIB@ \
	@ZOOKEEPER_LIB@ \
	@APR_LIB@ \
	@APRUTIL_LIB@ \
	@EXPAT_LIB@ \
	@LIB_THREAD_OS@
example2_LDADD = \
	$(top_builddir)/src/core/libcluster.a \
	@LOG4CXX_LIB@ \
	@ZOOKEEPER_LIB@ \
	@APR_LIB@ \
	@APRUTIL_LIB@ \
	@EXPAT_LIB@ \
	@LIB_THREAD_OS@
example_SOURCES = \
	example.cc
example2_SOURCES = \
	example2.cc

CXXFLAGS = @BASE_CXXFLAGS@

@makefile_inc@
