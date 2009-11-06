INCLUDES = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/src \
	-I@LOG4CXX_INCLUDES@ \
	-I@ZOOKEEPER_INCLUDES@ \
	-I@BOOST_INCLUDES@ \
	-I@MPICH_INCLUDES@ \
        -I@CPPUNIT_INCLUDES@
METASOURCES = AUTO
bin_PROGRAMS = unittests
unittests_LDADD = \
	$(top_builddir)/src/libcluster.a \
	$(top_builddir)/src/md5/libmd5.a \
	@LOG4CXX_LIB@ \
	@ZOOKEEPER_LIB@ \
	@CPPUNIT_LIB@ \
	@APR_LIB@ \
	@APRUTIL_LIB@ \
	@EXPAT_LIB@ \
	@LIB_THREAD_OS@ \
	@MPICH_LIB@
CXXFLAGS = @BASE_CXXFLAGS@ -DMPICH_IGNORE_CXX_SEEK 
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

@makefile_inc@
