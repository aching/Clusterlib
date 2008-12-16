INCLUDES = \
	-I$(top_srcdir)/common \
	-I@LOG4CXX_INCLUDES@ \
	-I@ZOOKEEPER_INCLUDES@ \
	-I@BOOST_INCLUDES@ \
	-I@MPICH_INCLUDES@
METASOURCES = AUTO
bin_PROGRAMS = test
test_LDADD = \
	$(top_builddir)/common/libcluster.a \
	@LOG4CXX@ \
	@ZOOKEEPER@ \
	@CPPUNIT_LIBS@ \
	@MPICH_LIB@
AM_CXXFLAGS = -Wall -g 
test_CXXFLAGS = $(CPPUNIT_CFLAGS) -DMPICH_IGNORE_CXX_SEEK
test_SOURCES = \
	test.cc \
	clusterlibProperties.cc 

@makefile_inc@
