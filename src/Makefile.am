INCLUDES = \
	-I@LOG4CXX_INCLUDES@ \
	-I@BOOST_INCLUDES@ \
	-I@ZOOKEEPER_INCLUDES@ \
        -I@top_srcdir@/include
METASOURCES = AUTO
lib_LIBRARIES = libcluster.a
libcluster_a_SOURCES = \
	md5key.cc \
	callbackandcontext.cc \
	notifyableimpl.cc \
	nodeimpl.cc \
	groupimpl.cc \
	applicationimpl.cc \
	datadistributionimpl.cc \
	propertiesimpl.cc \
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
