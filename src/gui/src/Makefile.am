INCLUDES = \
	-I. \
	-I@LOG4CXX_INCLUDES@ \
	-I@BOOST_INCLUDES@ \
	-I$(top_srcdir)/src/include \
	-I@ZOOKEEPER_INCLUDES@ \
	-I@MICROHTTPD_INCLUDES@ \
	-I@APR_INCLUDES@ \
	-I@APRUTIL_INCLUDES@
METASOURCES = AUTO
bin_PROGRAMS = zkuiserver
zkuiserver_LDADD = \
        $(top_builddir)/src/core/libcluster.a \
	$(NEW_BUILDSTAMP_OBJECT)\
	@BOOST_LIB@ \
        @ZOOKEEPER_LIB@ \
        @LOG4CXX_LIB@ \
        @MICROHTTPD_LIB@ \
	@APR_LIB@ \
	@APRUTIL_LIB@ \
	@EXPAT_LIB@ \
	@LIB_THREAD_OS@ \
	@LIBRT@
zkuiserver_SOURCES = \
	main.cc \
	xmlconfig.cc \
	httpd.cc \
	mhd_httpd.cc \
	jsonrpc_httpd_adaptor.cc \
	clusterlib_jsonrpc_adaptor.cc \
	zookeeper_jsonrpc_adaptor.cc \
	jsonrpc.cc
CXXFLAGS = -Wall -g @CXXFLAGS@

@makefile_inc@
