CONFIGURE_OPTS_opt := --with-ccache --enable-release

FLAVORS := opt

USE_COMMON_CONFIG := 1

include /usr/releng/share/build_scripts/ci.mk

export PATH := $(JAVA_HOME)/bin:$(PATH)

############################################################

build_flavor:
	mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && ../configure $(CONFIGURE_OPTS_$(FLAVOR))
	$(MAKE) -C $(BUILD_DIR)

package:
	rm -f $(BUILD_DIR)/*.tar.gz
	$(MAKE) -C $(BUILD_DIR) tarball

test:
	ulimit -n 1024; $(MAKE) -C $(BUILD_DIR) test_tinderbox
