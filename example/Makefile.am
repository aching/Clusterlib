
METASOURCES = AUTO
bin_PROGRAMS = example
example_LDADD = \
	$(top_builddir)/common/libcluster.a \
	@LOG4CXX@ \
	@ZOOKEEPER@
AM_CXXFLAGS = -Wall
INCLUDES = \
	-I$(top_srcdir)/common \
	-I@LOG4CXX_INCLUDES@ \
	-I@ZOOKEEPER_INCLUDES@ \
	-I@BOOST_INCLUDES@

example_SOURCES = \
	example.cc

@makefile_inc@
