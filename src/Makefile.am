INCLUDES = \
	-I@LOG4CXX_INCLUDES@ \
	-I@BOOST_INCLUDES@ \
	-I@ZOOKEEPER_INCLUDES@ \
        -I@top_srcdir@/include
METASOURCES = AUTO
bin_PROGRAMS = activenode
activenode_LDADD = \
        libcluster.a \
	$(top_builddir)/src/md5/libmd5.a \
	@LOG4CXX_LIB@ \
	@ZOOKEEPER_LIB@ \
	@CPPUNIT_LIB@ \
	@APR_LIB@ \
	@APRUTIL_LIB@ \
	@EXPAT_LIB@ \
	@LIB_THREAD_OS@
activenode_SOURCES = \
	activenode.cc \
	activenodeparams.cc
lib_LIBRARIES = libcluster.a
libcluster_a_SOURCES = \
	json.cc \
	processslotimpl.cc \
	md5key.cc \
	callbackandcontext.cc \
	notifyableimpl.cc \
	nodeimpl.cc \
	groupimpl.cc \
	applicationimpl.cc \
	datadistributionimpl.cc \
	propertylistimpl.cc \
	rootimpl.cc \
	internalchangehandlers.cc \
	cachedobjectchangehandlers.cc \
	distributedlocks.cc \
	clientimpl.cc \
	clusterlibstrings.cc \
	clusterlibints.cc \
	factory.cc \
	factoryops.cc \
	event.cc \
	mutex.cc \
        notifyablekeymanipulator.cc \
	signalmap.cc \
	thread.cc \
	zkadapter.cc
SUBDIRS = md5

CXXFLAGS = @BASE_CXXFLAGS@

@makefile_inc@
