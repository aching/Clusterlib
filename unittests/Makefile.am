INCLUDES = \
	-I$(top_srcdir)/src/include \
	-I$(top_srcdir)/src/core \
	-I@LOG4CXX_INCLUDES@ \
	-I@ZOOKEEPER_INCLUDES@ \
	-I@BOOST_INCLUDES@ \
	-I@MPICH_INCLUDES@ \
        -I@CPPUNIT_INCLUDES@
METASOURCES = AUTO
# Only build the unittests and libraries if MPICH and cppunit are present
if HAVE_MPICH
if HAVE_CPPUNIT
lib_LIBRARIES = libunittests.a
bin_PROGRAMS = unittests
endif
endif
libunittests_a_SOURCES = \
	testparams.cc
unittests_LDADD = \
	$(top_builddir)/src/core/libcluster.a \
	@LOG4CXX_LIB@ \
	@ZOOKEEPER_LIB@ \
	@CPPUNIT_LIB@ \
	@APR_LIB@ \
	@APRUTIL_LIB@ \
	@EXPAT_LIB@ \
	@LIB_THREAD_OS@ \
	@MPICH_LIB@
unittests_SOURCES = \
	test.cc \
	testparams.cc \
	clusterlibnode.cc \
	clusterliblock.cc \
	clusterlibjsonrpc.cc \
	clusterlibhierarchy.cc \
	clusterlibqueue.cc \
	clusterlibprocessslot.cc \
	clusterlibintervaltree.cc \
	clusterlibendevent.cc \
	clusterlibclient.cc \
	clusterlibpropertylist.cc \
	clusterlibuserevents.cc \
	clusterlibnotifyablekeymanipulator.cc \
	clusterlibdatadistribution.cc \
	clusterlibleader.cc \
	clusterlibtimer.cc \
	clusterlibhealthcheck.cc \
	clusterlibreleaseref.cc \
	clusterlibremove.cc
CXXFLAGS = @BASE_CXXFLAGS@ -DMPICH_IGNORE_CXX_SEEK -Wno-non-virtual-dtor

@makefile_inc@
