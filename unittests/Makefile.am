AM_CPPFLAGS = \
	-I$(top_srcdir)/src/include \
	-I$(top_srcdir)/src/core
AM_CXXFLAGS = @GENERAL_CXXFLAGS@ -DMPICH_IGNORE_CXX_SEEK -Wno-non-virtual-dtor
# Make sure to use the same CXX, not 'c++' by default in mpicxx
CXX = @MPICXX@ -cxx=@CXX@
# Only build the unittests and libraries if mpicxx and cppunit are present
if BUILD_UNITTESTS
lib_LTLIBRARIES = libunittests.la
bin_PROGRAMS = unittests
unittestsdir = $(includedir)/unittests
dist_unittests_HEADERS = \
	MPITestFixture.h \
	testparams.h
scriptsdir = $(prefix)/scripts
dist_scripts_DATA = unittestsHelper.pl.in
endif
libunittests_la_SOURCES = \
	testparams.cc
unittests_LDADD = \
	libunittests.la \
	$(top_builddir)/src/core/libcluster.la
unittests_SOURCES = \
	test.cc \
	clusterlibnode.cc \
	clusterliblock.cc \
	clusterlibjsonrpc.cc \
	clusterlibhierarchy.cc \
	clusterlibqueue.cc \
	clusterlibprocessslot.cc \
	clusterlibintervaltree.cc \
	clusterlibprocessthreadservice.cc \
	clusterlibendevent.cc \
	clusterlibclient.cc \
	clusterlibpropertylist.cc \
	clusterlibuserevents.cc \
	clusterlibnotifyablekeymanipulator.cc \
	clusterlibdatadistribution.cc \
	clusterlibleader.cc \
	clusterlibtimer.cc \
	clusterlibhealthcheck.cc \
	clusterlibremove.cc
