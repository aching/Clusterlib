CONFIGURE_OPTS_opt := --with-ccache --enable-release --with-zookeeper-jars-path=/usr/releng/internal/zookeeper/zookeeper-1.0.0/lib

FLAVORS := opt

# Don't use USE_COMMON_CONFIG since AUTOCONF and AUTOMAKE are not used
# individually to build this project
# USE_COMMON_CONFIG := 1

include /usr/releng/share/build_scripts/ci.mk

ARTIFACTS := $(BUILD_DIR)/*.tar.gz

UNITTEST_OUTPUT := $(BUILD_DIR)/result.xml

# Mpich bin path needed for unittests
export PATH := $(JAVA_HOME)/bin:$(PATH):/usr/releng/external/mpich2/mpich2-latest/bin

# Autotools environment
export AUTOCONF=/usr/releng/tools/autoconf/2.59/bin/autoconf 
export AUTOHEADER=/usr/releng/tools/autoconf/2.59/bin/autoheader
export AUTOMAKE=/usr/releng/tools/automake/1.9.4/bin/automake
export ACLOCAL=/usr/releng/tools/automake/1.9.4/bin/aclocal
export LIBTOOLIZE=/usr/releng/tools/libtool/1.5.10/bin/libtoolize

# Build environment
export CXX=/usr/releng/tools/gcc/4.1.1/bin/g++
export CXXFLAGS=-static
export CPPFLAGS=-I/usr/releng/external/boost/1.39.0_gcc411/include/boost-1_39 -I/usr/releng/internal/zookeeper/zookeeper-1.0.0/include -I/usr/releng/external/log4cxx/0.10.0_gcc411/include -I. -I/usr/releng/external/libmicrohttpd/0.4.0_gcc411/include -I/usr/releng/external/apr/1.2.12/include/apr-1 -I/usr/releng/external/apr-util/1.2.12/include/apr-1 -I/usr/releng/external/mpich2/mpich2-latest/install/include -I/usr/releng/external/cppunit/1.12.0_gcc411/include
export LDFLAGS=-L/usr/releng/external/boost/1.39.0_gcc411/lib -L/usr/releng/internal/zookeeper/zookeeper-1.0.0/lib -L/usr/releng/external/log4cxx/0.10.0_gcc411/lib -L/usr/releng/external/libmicrohttpd/0.4.0_gcc411/lib -L/usr/releng/external/apr/1.2.12/lib -L/usr/releng/external/apr-util/1.2.12/lib -L/usr/releng/external/mpich2/mpich2-latest/install/lib -L/usr/releng/external/cppunit/1.12.0_gcc411/lib -L/usr/local/lib/


############################################################

build_flavor:
	/usr/releng/tools/autoconf/2.59/bin/autoreconf -if
	mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && $(SRC_DIR)/configure $(CONFIGURE_OPTS_$(FLAVOR))
	$(MAKE) -C $(BUILD_DIR)

package:
	rm -f $(BUILD_DIR)/*.tar.gz
	$(MAKE) -C $(BUILD_DIR) tarball
	new_fn=`ls -1 $(BUILD_DIR)/*.tar.gz | sed -e 's@.tar.gz@.$(ARCH).tar.gz@'`; \
	mv -f $(BUILD_DIR)/*.tar.gz $$new_fn

test:
	ulimit -n 1024; $(MAKE) -C $(BUILD_DIR) TAP_PREFIX= check
