AM_CPPFLAGS = \
	-I. \
	-I$(top_srcdir)/src/include \
	-I$(top_srcdir)/src/gui/src 
AM_CXXFLAGS = @GENERAL_CXXFLAGS@
if BUILD_GUI_UNITTESTS
bin_PROGRAMS = zkuitest
endif
zkuitest_LDADD = \
	$(top_builddir)/src/core/libcluster.la \
        $(top_builddir)/src/gui/src/libzkui.la
zkuitest_SOURCES = \
	test.cc \
	jsontest.cc \
	jsonrpctest.cc \
	util.cc
