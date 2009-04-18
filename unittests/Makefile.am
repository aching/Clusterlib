INCLUDES = \
	-I$(top_srcdir)/common \
	-I@LOG4CXX_INCLUDES@ \
	-I@ZOOKEEPER_INCLUDES@ \
	-I@BOOST_INCLUDES@ \
	-I@MPICH_INCLUDES@
METASOURCES = AUTO
bin_PROGRAMS = unittests
unittests_LDADD = \
	$(top_builddir)/common/libcluster.a \
	@LOG4CXX_LIB@ \
	@ZOOKEEPER_LIB@ \
	@CPPUNIT_LIBS@ \
	@APR_LIB@ \
	@APRUTIL_LIB@ \
	@EXPAT_LIB@ \
	@LIB_THREAD_OS@ \
	@MPICH_LIB@
CXXFLAGS = @BASE_CXXFLAGS@ $(CPPUNIT_CFLAGS) -DMPICH_IGNORE_CXX_SEEK 
unittests_SOURCES = \
	test.cc \
	testparams.cc \
	clusterlibCache.cc \
	clusterlibClient.cc \
	clusterlibProperties.cc \
	clusterlibRemove.cc \
	clusterlibHierarchy.cc \
	clusterlibNotifyableKeyManipulator.cc

@makefile_inc@
