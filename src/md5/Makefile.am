INCLUDES = 
METASOURCES = AUTO
bin_PROGRAMS = md5
lib_LIBRARIES = libmd5.a
libmd5_a_SOURCES = \
	md5.cc
md5_LDADD = $(top_builddir)/src/md5/libmd5.a
md5_DEPENDENCIES = libmd5.a
md5_SOURCES = driver.cc

CXXFLAGS = @BASE_CXXFLAGS@

@makefile_inc@
