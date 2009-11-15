#
# Makefile.inc - Dreadnought specific Makefile targets
#
# Revision: $Id: Makefile.inc,v 1.4 2007-08-02 15:44:35 ckunz Exp $
#

# Rebuild the Makefile when this file changes
#
Makefile: $(top_srcdir)/Makefile.inc

# Packaging targets
#
package: $(top_builddir)/Manifest
	devpackage $(top_builddir)/Manifest $(top_builddir)/install
	rm -rf $(top_builddir)/Package
	ln -s  $(top_builddir)/install $(top_builddir)/Package

install: package

# Misc targets
#
maintainer-clean-local:
	-$(RM) -rf Makefile.in

maintainer-clean: maintainer-clean-local

clean-local:
	-$(RM) -rf $(BUILDSTAMP_SOURCE) $(BUILDSTAMP_OBJECT) $(NEW_BUILDSTAMP_SOURCE) $(NEW_BUILDSTAMP_OBJECT) gmon.out .deps *~

clean: clean-local

$(PROGRAMS) : $(NEW_BUILDSTAMP_OBJECT)

$(NEW_BUILDSTAMP_OBJECT) : $(NEW_BUILDSTAMP_SOURCE)
	$(NEW_MAKE_BUILDSTAMP_OBJECT)

$(NEW_BUILDSTAMP_SOURCE) : $(SOURCES) $(LIBRARIES)
	$(NEW_MAKE_BUILDSTAMP_SOURCE)


