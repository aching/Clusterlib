INCLUDES = \
	-I@LOG4CXX_INCLUDES@ \
	-I@BOOST_INCLUDES@ \
	-I@ZOOKEEPER_INCLUDES@
METASOURCES = AUTO
lib_LIBRARIES = libcluster.a
libcluster_a_SOURCES = \
	notifyableimpl.cc \
	nodeimpl.cc \
	groupimpl.cc \
	applicationimpl.cc \
	datadistributionimpl.cc \
	propertiesimpl.cc \
	rootimpl.cc \
	cachedobjectchangehandlers.cc \
	clientimpl.cc \
	serverimpl.cc \
	clusterlibstrings.cc \
	clusterlibints.cc \
	factory.cc \
	factoryops.cc \
	event.cc \
        notifyablekeymanipulator.cc \
	thread.cc \
	zkadapter.cc

CXXFLAGS = @BASE_CXXFLAGS@

@makefile_inc@
