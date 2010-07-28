INCLUDES = \
	-I. \
	-I$(top_srcdir)/src/include \
	-I$(top_srcdir)/src/core \
	-I@LOG4CXX_INCLUDES@ \
	-I@BOOST_INCLUDES@ \
	-I$(top_srcdir)/src/include \
	-I@ZOOKEEPER_INCLUDES@ \
	-I@MICROHTTPD_INCLUDES@ \
	-I@APR_INCLUDES@ \
	-I@APRUTIL_INCLUDES@
METASOURCES = AUTO
lib_LIBRARIES = libzkui.a
libzkui_a_SOURCES = \
	xmlconfig.cc \
	httpd.cc \
	microhttpserver.cc \
	jsonrpc_httpd_adaptor.cc \
	clusterlib_jsonrpc_adaptor.cc \
	zookeeper_jsonrpc_adaptor.cc \
	zookeeperuiserver.cc	
bin_PROGRAMS = zkuiserver
zkuiserver_LDADD = \
        $(top_builddir)/src/core/libcluster.a \
	$(NEW_BUILDSTAMP_OBJECT) \
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
	microhttpserver.cc \
	jsonrpc_httpd_adaptor.cc \
	clusterlib_jsonrpc_adaptor.cc \
	zookeeper_jsonrpc_adaptor.cc \
	zookeeperuiserver.cc

CXXFLAGS = @BASE_CXXFLAGS@

@makefile_inc@
