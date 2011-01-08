# Process this file with autoreconf to generate a configure script
AC_INIT([clusterlib], [0.9])

# Use Automake for writing Makefiles
AM_INIT_AUTOMAKE([-Wall])

# Use libtool to generate libraries
# LT_PREREQ([1.5])

# Minimum Autoconf version
AC_PREREQ(2.59)

# Safety check to enure that the source directory is correct
AC_CONFIG_SRCDIR([src/include/clusterlib.h])

# Look for the C++ compiler
AC_PROG_CXX

# Find the install program
AC_PROG_INSTALL

# Look for the libtool
AC_PROG_LIBTOOL

# Enable large file support when possible
AC_SYS_LARGEFILE

# Allow per-product flags
AM_PROG_CC_C_O

# Use C++ for all compilation tests (otherwise defaults to gcc and all
# AC_CHECK_HEADER checks will fail for C++ libraries)
AC_LANG([C++])

# Boost regex header and library must exist (C++ style)
AC_CHECK_HEADER([boost/regex.hpp],,
  AC_MSG_ERROR([boost/regex.hpp not found.  Exiting.]))
AC_CHECK_LIB([boost_regex], [main],,
  AC_MSG_ERROR([boost_regex library not found.  Exiting.]))

# Check for pthread library
AC_SEARCH_LIBS([pthread_create], [pthread], [have_pthread=yes])

# Pthreads are required
if test "x${have_pthread}" = xno; then
  echo "------------------------------------------"
  echo " The pthread library and header file is   "
  echo " required to build clusterlib. Stopping..."
  echo " Check 'config.log' for more information. "
  echo "------------------------------------------"
  (exit 1); exit 1;
fi

# ZooKeeper header must exist and library must exist
AC_CHECK_HEADER([c-client-src/zookeeper.h],,
  AC_MSG_ERROR([c-client-src/zookeeper.h not found.  Exiting.]))
AC_SEARCH_LIBS([zookeeper_init], [zookeeper_mt], [have_zookeeper=yes],
  AC_MSG_ERROR([zookeeper_mt library not found.  Exiting.]))

# Log4cxx header must exist and library must exist
AC_CHECK_HEADER([log4cxx/logger.h],,
  AC_MSG_ERROR([log4cxx/logger.h not found.  Exiting.]))
AC_CHECK_LIB([log4cxx], [main],,
  AC_MSG_ERROR([log4cxx library not found.  Exiting.]))

# Ncurses is not required, but would allows the CLI to use tab-completion
AC_CHECK_LIB([ncurses], [tputs],,
  echo "------------------------------------------"
  echo " The ncurses library                      "
  echo " is required to get tab-completion.      "
  echo " The path should be specified in LDFLAGS. "
  echo " Check 'config.log' for more information. "
  echo "------------------------------------------"
)

# Readline is not required, but would allows the CLI to use tab-completion
AC_CHECK_HEADER([readline/readline.h],,
  echo "------------------------------------------"
  echo " The readline headers (i.e. readline/readline.h)"
  echo " required get tab-completion are missing. "
  echo " The path should be specified in CPPFLAGS."
  echo " Check 'config.log' for more information. "
  echo "------------------------------------------"
)
AC_CHECK_LIB([readline], [rl_line_buffer],,
  echo "------------------------------------------"
  echo " The readline library                     "
  echo " is required to get tab-completion.       "
  echo " Check 'config.log' for more information. "
  echo "------------------------------------------"
)

# Should build the GUI?
build_gui=true

# Apr-1 is not required, but would allow the GUI to be built.
# Check for the apr_getopt.h in either apr-1 or apr-1.0 (i.e. ubuntu)
AC_CHECK_FILE([/usr/include/apr-1/apr_getopt.h],
  CPPFLAGS="${CPPFLAGS} -I/usr/include/apr-1"
  AC_MSG_RESULT([Found /usr/include/apr-1/apr_getopt.h and adding /usr/include/apr-1 to CPPFLAGS.]))
AC_CHECK_FILE([/usr/include/apr-1.0/apr_getopt.h],
  CPPFLAGS="${CPPFLAGS} -I/usr/include/apr-1.0"
  AC_MSG_RESULT([Found /usr/include/apr-1.0/apr_getopt.h and adding /usr/include/apr-1.0 to CPPFLAGS.]))
AC_CHECK_HEADER([apr_getopt.h],,
  echo "------------------------------------------"
  echo " The apr-1 headers (i.e. apr_getopt.h)    "
  echo " required to build the GUI are missing.   "
  echo " The path should be specified in CPPFLAGS."
  echo " Check 'config.log' for more information. "
  echo "------------------------------------------"
  build_gui=false)
AC_CHECK_LIB([apr-1], [main],,
  echo "------------------------------------------"
  echo " The apr-1 library                        "
  echo " required to build the GUI is missing.    "
  echo " The path should be specified in LDFLAGS. "
  echo " Check 'config.log' for more information. "
  echo "------------------------------------------"
  build_gui=false)

# Aprutil-1 is not required, but would allow the GUI to be built.
# Check for the apr_xml.h in either apr-1 or apr-1.0 (i.e. ubuntu)
AC_CHECK_FILE([/usr/include/apr-1/apr_xml.h],
  CPPFLAGS="${CPPFLAGS} -I/usr/include/apr-1"
  AC_MSG_RESULT([Found /usr/include/apr-1/apr_xml.h and adding /usr/include/apr-1 to CPPFLAGS.]))
AC_CHECK_FILE([/usr/include/apr-1.0/apr_xml.h],
  CPPFLAGS="${CPPFLAGS} -I/usr/include/apr-1.0"
  AC_MSG_RESULT([Found /usr/include/apr-1.0/apr_xml.h and adding /usr/include/apr-1.0 to CPPFLAGS.]))
AC_CHECK_HEADER([apr_xml.h],,
  echo "------------------------------------------"
  echo " The aprutil-1 headers (i.e. apr_xml.h)   "
  echo " required to build the GUI are missing.   "
  echo " The path should be specified in CPPFLAGS."
  echo " Check 'config.log' for more information. "
  echo "------------------------------------------"
  build_gui=false)
AC_CHECK_LIB([aprutil-1], [main],,
  echo "------------------------------------------"
  echo " The aprutil-1 library                    "
  echo " required to build the GUI is missing.    "
  echo " The path should be specified in LDFLAGS. "
  echo " Check 'config.log' for more information. "
  echo "------------------------------------------"
  build_gui=false)

# Build the GUI unittests?
AC_ARG_ENABLE([gui_unittests],
  [AS_HELP_STRING([--enable-gui_unittests], 
    [build GUI unittests] [default=no])], 
  [build_gui_unittests=true], 
  [build_gui_unittests=false])

# Boost unit_test_framework header and library must exist for GUI unittests
if test x${build_gui_unittests} = xtrue; then
  AC_CHECK_HEADER([boost/test/auto_unit_test.hpp],,
    AC_MSG_ERROR([boost/test/auto_unit_test.hpp not found for GUI unittests.  Exiting.]))
  AC_CHECK_LIB([boost_unit_test_framework], [main],,
    AC_MSG_ERROR([boost_unit_test_framework library not found for GUI unittests.  Exiting.]))
fi

AM_CONDITIONAL(BUILD_GUI_UNITTESTS, [test "x${build_gui_unittests}" = x"true"])

# Microhttpd is not required, but would allow the GUI to be built.
AC_CHECK_HEADER([microhttpd.h],,
  echo "------------------------------------------"
  echo " The microhttpd headers (i.e. microhttpd.h)"
  echo " required to build the GUI are missing.   "
  echo " The path should be specified in CPPFLAGS."
  echo " Check 'config.log' for more information. "
  echo "------------------------------------------"
  build_gui=false)
AC_CHECK_LIB([microhttpd], [main],,
  echo "------------------------------------------"
  echo " The microhttpd library                    "
  echo " required to build the GUI is missing.    "
  echo " The path should be specified in LDFLAGS. "
  echo " Check 'config.log' for more information. "
  echo "------------------------------------------"
  build_gui=false)

# Microhttpd > 0.9 needs size_t max, otherwise int max
mhd_max_type=int
if test x${build_gui} = xtrue; then
  AC_COMPILE_IFELSE(
    [AC_LANG_PROGRAM([[
      #include <stdint.h>
      #include <stdlib.h>
      #include <stdio.h>
      #include <unistd.h>
      #include <microhttpd.h>
      ]],
      [[
      #if MHD_VERSION >= 0x00090000
      // MHD_VERSION >= 0x00090000 needs size_t max, not int max
      #else
      #  MHD_VERSION is too low
      #endif
      ]])],
    mhd_max_type=size_t
    [AC_MSG_RESULT([[The libmicrohttpd version is >= 0.9 and will use size_t max.]])],
    [AC_MSG_RESULT([[The libmicrohttpd version is < 0.9 and will use int max.]])]
  )
fi
if test "x${mhd_max_type}" = x"size_t"; then
  AC_DEFINE([MHD_MAX_SIZE_T], [1], [Use size_t for MHD max])
fi

AM_CONDITIONAL(BUILD_GUI, [test "x${build_gui}" = x"true"])

# Mpich binaries (mpiexec, mpd) are required for unittests to be run
AC_PATH_PROG([MPIEXEC], [mpiexec])
if test x${MPIEXEC} = x; then
  echo "------------------------------------------"
  echo " Mpiexec is required to run the           "
  echo " unittests and cannot be found.           "
  echo " Check 'config.log' for more information. "
  echo "------------------------------------------"
fi
AC_PATH_PROG([MPD], [mpd])
if test x${MPD} = x; then
  echo "------------------------------------------"
  echo " Mpd is required to run the               "
  echo " unittests and cannot be found.           "
  echo " Check 'config.log' for more information. "
  echo "------------------------------------------"
fi

# ZooKeeper jar file locations are required to run the unittests
AC_ARG_WITH([zookeeper_jars_path],
  [AS_HELP_STRING([--with-zookeeper-jars-path], 
    [location of ZooKeeper jar files])],
  [case "${withval}" in yes|no)
    AC_ERROR(Option --with-zookeeper-jars-path requires a path argument.)
        ;;
    esac 
  AC_MSG_RESULT([Using $with_zookeeper_jars_path for zookeeper jars path])
])
ZOOKEEPER_JARS_PATH="${with_zookeeper_jars_path}"
AC_SUBST(ZOOKEEPER_JARS_PATH)
if test x${ZOOKEEPER_JARS_PATH} = x; then
  echo "------------------------------------------"
  echo " Zookeeper jars path is not found and     "
  echo " needed for the unittests (start/stop zk)."
  echo " See option --with-zookeeper-jars-path.   "
  echo " Check 'config.log' for more information. "
  echo "------------------------------------------"
fi

# Should build the unittests?
build_unittests=true

AC_PATH_PROG([MPICXX], [mpicxx])
if test x${MPICXX} = x; then
  echo "------------------------------------------"
  echo " mpicxx is required to compile            "
  echo " unittests and cannot be found.           "
  echo " Check 'config.log' for more information. "
  echo "------------------------------------------"
  build_unittests=false
fi

# Cppunit is not required, but would allow the unittests to be built.
AC_CHECK_HEADER([cppunit/ui/text/TestRunner.h],,
  echo "------------------------------------------"
  echo " The cppunit headers (i.e. cppunit/ui/text/TestRunner.h)"
  echo " required to build unittests are missing. "
  echo " The path should be specified in CPPFLAGS."
  echo " Check 'config.log' for more information. "
  echo "------------------------------------------"
  build_unittests=false)
AC_CHECK_LIB([cppunit], [main],,
  echo "------------------------------------------"
  echo " The cppunit library                      "
  echo " required to build unittests is missing.  "
  echo " The path should be specified in LDFLAGS. "
  echo " Check 'config.log' for more information. "
  echo "------------------------------------------"
  build_unittests=false)

AM_CONDITIONAL(BUILD_UNITTESTS, [test "x${build_unittests}" = x"true"])

# Check to see if the following headers exist
AC_CHECK_HEADERS([pthread.h sys/epoll.h arpa/inet.h fcntl.h inttypes.h netdb.h stdlib.h string.h strings.h sys/socket.h sys/syscall.h sys/time.h unistd.h boost/test/auto_unit_test.hpp readline/readline.h apr_getopt.h apr_xml.h mach/mach.h])

# Generate a header file from configure with various configure output
AC_CONFIG_HEADERS([config.h])

# Check for various library functions
AC_CHECK_FUNCS([atexit bzero clock_gettime dup2 gethostname gettimeofday memset socket strdup strerror strtol mach_thread_self])

# Get the build path for doxygen
BUILD_PATH="`pwd`"
AC_SUBST(BUILD_PATH)

AC_CONFIG_FILES([
Makefile
src/Makefile
src/activenode/Makefile
src/cli/Makefile
src/core/Makefile
src/core/md5/Makefile
src/example/Makefile
src/gui/Makefile
src/gui/src/Makefile
src/gui/unittest/Makefile
src/gui/conf/config.xml
src/gui/conf/Makefile
src/include/Makefile
misc/Makefile
unittests/Makefile
unittests/unittestsHelper.pl
docs/doxygen-internal/doxygenInternalConfig
docs/doxygen-user/doxygenUserConfig])

# Default is release build, otherwise its debug
AC_ARG_ENABLE([debug],
  [AS_HELP_STRING([--enable-debug], [enable debug build] [default=no])], 
  [], [enable_debug=no])

# Disable Werror
AC_ARG_ENABLE([werror],
  [AS_HELP_STRING([--disable-werror], [allow warnings] [default=no])], 
  [], [disable_werror=no])

# Set some general CXXFLAGS 
GENERAL_CXXFLAGS="-Wall -g -rdynamic -D__STDC_FORMAT_MACROS -fno-strict-aliasing"
if test "x${enable_debug}" = xno; then
  GENERAL_CXXFLAGS="${GENERAL_CXXFLAGS} -O3"
else
  GENERAL_CXXFLAGS="${GENERAL_CXXFLAGS} -O0"
fi

# Add the Werror flags unless disabled
if test "x${disable_werror}" = xno; then
  GENERAL_CXXFLAGS="$GENERAL_CXXFLAGS -Werror"
fi
AC_SUBST(GENERAL_CXXFLAGS)

# No extra CXXFLAGS should be added if none were specified
if test "x$ac_cv_env_CXXFLAGS_set" = x; then
  CXXFLAGS=""
fi

if test "x${ac_cv_sys_file_offset_bits}" = xno; then
  AC_DEFINE([_DARWIN_FEATURE_64_BIT_INODE], [1], [Make sure stat is 64 bit])
fi

AC_OUTPUT
