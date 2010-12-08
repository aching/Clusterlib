AM_CPPFLAGS = -I$(top_srcdir)/src/include
AM_CXXFLAGS = @GENERAL_CXXFLAGS@
noinst_PROGRAMS = example example2
example_LDADD = \
	$(top_builddir)/src/core/libcluster.la 
example2_LDADD = \
	$(top_builddir)/src/core/libcluster.la 
example_SOURCES = \
	example.cc
example2_SOURCES = \
	example2.cc
