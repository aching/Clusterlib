AM_CPPFLAGS = \
	-I. \
	-I$(top_srcdir)/src/include \
	-I$(top_srcdir)/src/core
AM_CXXFLAGS = @GENERAL_CXXFLAGS@
lib_LTLIBRARIES = libzkui.la
libzkui_la_SOURCES = \
	xmlconfig.cc \
	httpd.cc \
	microhttpserver.cc \
	jsonrpc_httpd_adaptor.cc \
	clusterlib_jsonrpc_adaptor.cc \
	zookeeper_jsonrpc_adaptor.cc \
	zookeeperuiserver.cc	
bin_PROGRAMS = zkuiserver
zkuiserver_LDADD = \
        libzkui.la \
        $(top_builddir)/src/core/libcluster.la
zkuiserver_SOURCES = \
	main.cc
guidir = $(includedir)/gui
dist_gui_HEADERS = \
	clusterlib_jsonrpc_adaptor.h \
	gui.h \
	httpd.h \
	jsonrpc_httpd_adaptor.h \
	microhttpserver.h \
	xmlconfig.h \
	zookeeper_jsonrpc_adaptor.h \
	zookeeperuiserver.h
