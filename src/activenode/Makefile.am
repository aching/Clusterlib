INCLUDES = \
	-I$(top_srcdir)/src/include \
	-I$(top_srcdir)/src/core \
	-I@LOG4CXX_INCLUDES@ \
	-I@BOOST_INCLUDES@ \
	-I@ZOOKEEPER_INCLUDES@
METASOURCES = AUTO
bin_PROGRAMS = activenode
activenode_LDADD = \
        $(top_builddir)/src/core/libcluster.a \
	@LOG4CXX_LIB@ \
	@ZOOKEEPER_LIB@ \
	@APR_LIB@ \
	@APRUTIL_LIB@ \
	@EXPAT_LIB@ \
	@LIB_THREAD_OS@
activenode_SOURCES = \
        activenodejsonrpcadaptor.cc \
	activenode.cc \
	activenodeparams.cc 

CXXFLAGS = @BASE_CXXFLAGS@

@makefile_inc@
