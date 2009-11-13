# not a GNU package. You can remove this line, if
# have all needed files, that a GNU package needs
AUTOMAKE_OPTIONS = 1.3 foreign no-installman no-installinfo no-dependencies 

SUBDIRS = src unittests
LDADD   = $(NEW_BUILDSTAMP_OBJECT)

PKGDIR = @PACKAGE@_@VERSION@.@OS_TYPE@

###
# Packaging targets

package_clusterlib: $(top_builddir)/Manifest
	devpackage $(top_builddir)/Manifest $(top_builddir)/install

documentation:
	doxygen $(top_builddir)/docs/doxygen-internal/doxygenInternalConfig
	doxygen $(top_builddir)/docs/doxygen-user/doxygenUserConfig

package: all package_clusterlib

install: package

tarball: package documentation
	mkdir install/docs
	cp -r $(top_builddir)/docs/doxygen-internal install/docs
	cp -r $(top_builddir)/docs/doxygen-user install/docs
	mv install $(PKGDIR)
	if test -f $(PKGDIR)/BoM.tar.gz; \
		then /usr/releng/bin/gtar -czf $(PKGDIR).tar.gz $(PKGDIR)/*; \
		else /usr/releng/bin/gtar -czf $(PKGDIR).tar.gz $(PKGDIR)/*;\
	fi;
	mv $(PKGDIR) install

test: package
	inktest -a -b $(top_builddir) -c $(top_builddir)/regressions/Regressions.itc \
                --verbose
