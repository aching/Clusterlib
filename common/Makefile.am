INCLUDES = \
	-I. \
	-I@SIMON_INCLUDES@ \
	-I@LOG4CXX_INCLUDES@ \
	-I@BOOST_INCLUDES@ \
	-I@ZOOKEEPER_INCLUDES@ \
	-I@CDXCORE_INCLUDES@/libcrawdexcore \
	-I@CDXCORE_INCLUDES@/libcrawdex \
	-I@CDXCORE_INCLUDES@/libinktomi \
	-I@CDXCORE_INCLUDES@/basics \
	-I@CDXCORE_INCLUDES@/basics/atoms \
	-I@CPPUTILS_INCLUDES@
METASOURCES = AUTO
lib_LIBRARIES = libcluster.a
libcluster_a_SOURCES = \
	event.cc \
	hash.cc \
	log.cc \
	thread.cc \
	zkadapter.cc
AM_CXXFLAGS = -Wall @CDXCORE_CXXFLAGS@
