# not a GNU package. You can remove this line, if
# have all needed files, that a GNU package needs
AUTOMAKE_OPTIONS = 1.3 foreign no-installman no-installinfo no-dependencies 

SUBDIRS = src misc unittests

PKGDIR = @PACKAGE@_@VERSION@-release

# Packaging targets

package_clusterlib:
	make prefix=@BUILD_PATH@/install install

documentation:
	doxygen $(top_builddir)/docs/doxygen-internal/doxygenInternalConfig
	doxygen $(top_builddir)/docs/doxygen-user/doxygenUserConfig

package: all package_clusterlib

tarball: package documentation
	if [ ! -d "install/docs" ]; then mkdir install/docs; fi
	cp -u -r $(top_builddir)/docs/doxygen-internal install/docs
	cp -u -r $(top_builddir)/docs/doxygen-user install/docs
	cp -u -r $(top_srcdir)/src/gui/webroot install/webroot
	mv install $(PKGDIR)
	if test -f $(PKGDIR)/BoM.tar.gz; \
		then tar -czf $(PKGDIR).tar.gz $(PKGDIR)/*; \
		else tar -czf $(PKGDIR).tar.gz $(PKGDIR)/*;\
	fi;
	mv $(PKGDIR) install

check: all
	chmod +x $(top_builddir)/unittests/unittestsHelper.pl	
	$(top_builddir)/unittests/unittestsHelper.pl --testBin $(top_builddir)/unittests/unittests --numProcs 2
