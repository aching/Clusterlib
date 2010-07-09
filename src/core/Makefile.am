INCLUDES = \
	-I@LOG4CXX_INCLUDES@ \
	-I@BOOST_INCLUDES@ \
	-I@ZOOKEEPER_INCLUDES@ \
        -I@top_srcdir@/src/include
METASOURCES = AUTO
lib_LIBRARIES = libcluster.a
libcluster_a_SOURCES = \
	cacheddataimpl.cc \
	cachedstateimpl.cc \
	cachedkeyvaluesimpl.cc \
	cachedprocessinfoimpl.cc \
	cachedprocessslotinfoimpl.cc \
	cachedshardsimpl.cc \
	registerednotifyableimpl.cc \
	registeredrootimpl.cc \
	registeredapplicationimpl.cc \
	registeredgroupimpl.cc \
	registereddatadistributionimpl.cc \
	registerednodeimpl.cc \
	registeredqueueimpl.cc \
	registeredprocessslotimpl.cc \
	registeredpropertylistimpl.cc \
	safenotifyablemap.cc \
	processthreadservice.cc \
	clusterlibexceptions.cc \
	unknownhashrange.cc \
	json.cc \
	jsonrpc.cc \
	queueimpl.cc \
	processslotimpl.cc \
	callbackandcontext.cc \
	shard.cc \
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
	zkadapter.cc \
	jsonrpcresponsehandler.cc \
	jsonrpcmethodhandler.cc \
	genericrpc.cc \
	clusterlibrpc.cc \
	clusterlibrpcbulkrequest.cc \
	zookeeperperiodiccheck.cc \
        md5/md5.cc
SUBDIRS = md5

# Special target for md5 directory cc files to be included in libcluster.a
%.o: md5/%.cc
	$(CXXCOMPILE) -c $<

CXXFLAGS = @BASE_CXXFLAGS@

@makefile_inc@
