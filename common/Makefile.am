INCLUDES = \
	-I@LOG4CXX_INCLUDES@ \
	-I@BOOST_INCLUDES@ \
	-I@ZOOKEEPER_INCLUDES@
METASOURCES = AUTO
lib_LIBRARIES = libcluster.a
libcluster_a_SOURCES = \
	application.cc \
	clusterclient.cc \
	clusterlib.cc \
	clusterserver.cc \
	datadistribution.cc \
	properties.cc \
	notifyable.cc \
	event.cc \
	group.cc \
	log.cc \
	node.cc \
	notifyable.cc \
	thread.cc \
	zkadapter.cc
CXXFLAGS = @BASE_CXXFLAGS@
