# not a GNU package. You can remove this line, if
# have all needed files, that a GNU package needs
AUTOMAKE_OPTIONS = foreign 1.4

SUBDIRS = common example unittests

PKGDIR = @PACKAGE@_@VERSION@.@OS_TYPE@

###
# Packaging targets

package_clusterlib: $(top_builddir)/Manifest
	devpackage $(top_builddir)/Manifest $(top_builddir)/install

package: all package_clusterlib

install: package

tarball: package
	mv install $(PKGDIR)
	if test -f $(PKGDIR)/BoM.tar.gz; \
		then /usr/releng/bin/gtar -czf $(PKGDIR).tar.gz $(PKGDIR)/*; \
		else /usr/releng/bin/gtar -czf $(PKGDIR).tar.gz $(PKGDIR)/*;\
	fi;
	mv $(PKGDIR) install

test_tinderbox: package
	inktest -a -b $(top_builddir) -c $(srcdir)/regressions/Regressions.itc \
                --verbose
