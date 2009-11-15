INCLUDES = \
	-I$(top_srcdir)/src/include \
	-I$(top_srcdir)/src/core \
	-I@LOG4CXX_INCLUDES@ \
	-I@ZOOKEEPER_INCLUDES@ \
	-I@BOOST_INCLUDES@ \
	-I@MPICH_INCLUDES@ \
        -I@CPPUNIT_INCLUDES@
METASOURCES = AUTO

# Only build the unittests if MPICH and cppunit are present
if HAVE_MPICH
if HAVE_CPPUNIT
bin_PROGRAMS = unittests
endif
endif
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
	clusterlibprocessslot.cc \
	clusterlibintervaltree.cc \
	clusterlibdatadistribution.cc \
	clusterlibendevent.cc \
	clusterlibcache.cc \
	clusterlibclient.cc \
	clusterlibpropertylist.cc \
	clusterlibremove.cc \
	clusterlibhierarchy.cc \
	clusterlibuserevents.cc \
	clusterlibnotifyablekeymanipulator.cc \
	clusterlibleader.cc \
	clusterlibtimer.cc \
	clusterlibhealthcheck.cc \
	clusterlibreleaseref.cc

CXXFLAGS = @BASE_CXXFLAGS@ -DMPICH_IGNORE_CXX_SEEK 

@makefile_inc@
