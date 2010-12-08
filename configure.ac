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

# Boost unit_test_framework header and library must exist (C++ style)
AC_CHECK_HEADER([boost/test/auto_unit_test.hpp],,
  AC_MSG_ERROR([boost/test/auto_unit_test.hpp not found.  Exiting.]))
AC_CHECK_LIB([boost_unit_test_framework], [main],,
  AC_MSG_ERROR([boost_unit_test_framework library not found.  Exiting.]))

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
  echo " The ncurses library and header files     "
  echo " are required to get tab-completion.      "
  echo " Check 'config.log' for more information. "
  echo "------------------------------------------"
)

# Readline is not required, but would allows the CLI to use tab-completion
AC_CHECK_LIB([readline], [rl_line_buffer],,
  echo "------------------------------------------"
  echo " The readline library and header files    "
  echo " are required to get tab-completion.      "
  echo " Check 'config.log' for more information. "
  echo "------------------------------------------"
)

# Should build the GUI?
build_gui=true;

# Apr-1 is not required, but would allow the GUI to be built.
AC_CHECK_HEADER([apr_getopt.h],
  AC_CHECK_LIB([apr-1], [main]))
if test x${ac_cv_lib_apr_1_main} = xno; then
  echo "------------------------------------------"
  echo " The apr library and header files         "
  echo " are required to build the GUI.           "
  echo " Check 'config.log' for more information. "
  echo "------------------------------------------"
  build_gui=false  
fi

# Aprutil-1 is not required, but would allow the GUI to be built.
AC_CHECK_HEADER([apu.h],
  AC_CHECK_LIB([aprutil-1], [main]))
if test x${ac_cv_lib_aprutil_1_main} = xno; then
  echo "------------------------------------------"
  echo " The aprutil-1 library and header files   "
  echo " are required to build the GUI.           "
  echo " Check 'config.log' for more information. "
  echo "------------------------------------------"
  build_gui=false  
fi

# Microhttpd is not required, but would allow the GUI to be built.
AC_CHECK_HEADER([microhttpd.h],
  AC_CHECK_LIB([microhttpd], [main]))
if test x${ac_cv_lib_microhttpd_main} = xno; then
  echo "------------------------------------------"
  echo " The microhttpd library and header files  "
  echo " are required to build the GUI.           "
  echo " Check 'config.log' for more information. "
  echo "------------------------------------------"
  build_gui=false;
fi

AM_CONDITIONAL(BUILD_GUI, [test "x${build_gui}" = x"true"])

# Mpich binaries are required for unittests to be run
AC_PATH_PROG([MPIEXEC], mpiexec)
if test x${MPIEXEC} = x; then
  echo "------------------------------------------"
  echo " Mpiexec is required to run the           "
  echo " unittests and cannot be found.           "
  echo " Check 'config.log' for more information. "
  echo "------------------------------------------"
  build_gui=false;
fi
AC_PATH_PROG([MPD], mpd)
if test x${MPD} = x; then
  echo "------------------------------------------"
  echo " Mpiexec is required to run the           "
  echo " unittests and cannot be found.           "
  echo " Check 'config.log' for more information. "
  echo "------------------------------------------"
  build_gui=false;
fi

# ZooKeeper jar file locations are required to run the unittests
AC_ARG_WITH([zookeeper_jars_path],
  [AS_HELP_STRING([--with-zookeeper-jars-path], 
    [location of ZooKeeper jar files])],
  [case "${withval}" in yes|no)
    AC_ERROR(Option --with-zookeeper-jars-path requires a path argument.)
        ;;
    esac 
  AC_MSG_RESULT([Using $with_zookeeper_jars_path])
])
ZOOKEEPER_JARS_PATH="${with_zookeeper_jars_path}"
AC_SUBST(ZOOKEEPER_JARS_PATH)
if test x${ZOOKEEPER_JARS_PATH} = x; then
  echo "------------------------------------------"
  echo " Zookeeper jars path is not found and     "
  echo " needed for the unittests (start/stop zk)."
  echo " Check 'config.log' for more information. "
  echo "------------------------------------------"
fi

# Should build the unittests?
build_unittests=true;

# Mpich is not required, but would allow the unittests to be built.
AC_LANG_PUSH([C])
AC_CHECK_HEADER([mpi.h],
  AC_CHECK_LIB([mpich], [MPI_Init])
  if test x${ac_cv_lib_mpich_MPI_Init} = xno; then
    echo "------------------------------------------"
    echo " The mpich library and header files       "
    echo " are required to build the unittests.     "
    echo " Check 'config.log' for more information. "
    echo "------------------------------------------"
    build_unittests=false  
  fi
  AC_CHECK_LIB([mpichcxx], [main])
  if test x${ac_cv_lib_mpichcxx_main} = xno; then
    echo "------------------------------------------"
    echo " The mpichcxx library and header files    "
    echo " are required to build the unittests.     "
    echo " Check 'config.log' for more information. "
    echo "------------------------------------------"
    build_unittests=false  
  fi
)
AC_LANG_POP([C])

# Cppunit is not required, but would allow the unittests to be built.
AC_CHECK_HEADER([cppunit/ui/text/TestRunner.h],
  AC_CHECK_LIB([cppunit], [main]))
if test x${ac_cv_lib_cppunit_main} = xno; then
  echo "------------------------------------------"
  echo " The cppunit library and header files     "
  echo " are required to build the unittests.     "
  echo " Check 'config.log' for more information. "
  echo "------------------------------------------"
  build_unittests=false  
fi

AM_CONDITIONAL(BUILD_UNITTESTS, [test "x${build_unittests}" = x"true"])

# Check to see if the following headers exist
AC_CHECK_HEADERS([pthread.h linux/unistd.h sys/epoll.h arpa/inet.h fcntl.h inttypes.h netdb.h stdlib.h string.h strings.h sys/socket.h sys/time.h unistd.h boost/test/auto_unit_test.hpp readline/readline.h])

# Generate a header file from configure with various configure output
AC_CONFIG_HEADERS([config.h])


# Enable unittests?
#AC_ARG_ENABLE([core-unittests],
#  [  --enable-core-unittests  enable core cppunit tests],
#  [core_unittests=${enableval}],
#  [core_unittests=no])


# Check for mpi if trying to build unittests
#if test "x${core_unittests}" = xyes; then
#  AC_SEARCH_LIBS([mpi_init], [mpi], [have_mpi=yes])
#  echo "------------------------------------------"
#  echo " The mpi library and header file is       "
#  echo " required to build unittests. Stopping... "
#  echo " Check 'config.log' for more information. "
#  echo "------------------------------------------"
#  (exit 1); exit 1;
#else
#  AC_MSG_NOTICE([Core unittests disabled])
#fi

# Check for various library functions
AC_CHECK_FUNCS([atexit bzero clock_gettime dup2 gethostname gettimeofday memset socket strdup strerror strtol])

# Get the build path for doxygen
BUILD_PATH="`pwd`"
AC_SUBST(BUILD_PATH)

# Generate the following files
#AC_CONFIG_FILES([
#Manifest
#Makefile
#src/Makefile
#src/activenode/Makefile
#src/cli/Makefile
#src/core/Makefile
#src/core/md5/Makefile
#src/example/Makefile
#src/gui/Makefile
#src/gui/src/Makefile
#src/gui/unittest/Makefile
#src/gui/conf/config.xml
#unittests/Makefile
#unittests/unittestsHelper.pl
#regressions/Regressions.itc
#docs/doxygen-internal/doxygenInternalConfig
#docs/doxygen-user/doxygenUserConfig])
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

# Set some general CXXFLAGS 
if test "x${enable_debug}" = xno; then
   GENERAL_CXXFLAGS="-Wall -Werror -O3 -g -rdynamic -D__STDC_FORMAT_MACROS -fno-strict-aliasing"
else
   GENERAL_CXXFLAGS="-Wall -Werror -O0 -g -rdynamic -D__STDC_FORMAT_MACROS -fno-strict-aliasing"
fi
AC_SUBST(GENERAL_CXXFLAGS)

AC_OUTPUT
