# Only build if the microhttpd library is available
if HAVE_MICROHTTPD
SUBDIRS = src unittest
endif

@makefile_inc@

###
# Packaging targets

#package_zkui: $(top_builddir)/Manifest
#	devpackage $(top_builddir)/Manifest $(top_builddir)/install

#package: all package_zkui

#install: package

#tarball: package
#	mv install $(PKGDIR)
#	if test -f $(PKGDIR)/BoM.tar.gz; \
#		then /usr/releng/bin/gtar -czf $(PKGDIR).tar.gz $(PKGDIR)/*; \
#		else /usr/releng/bin/gtar -czf $(PKGDIR).tar.gz $(PKGDIR)/*;\
#	fi;
#	mv $(PKGDIR) install

#test_tinderbox: package
#	inktest -a -b $(top_builddir) -c $(srcdir)/regressions/Regressions.itc \
#                --verbose
