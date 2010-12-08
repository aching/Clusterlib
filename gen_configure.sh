#!/bin/sh

# Use this script to generate 'configure' from configure.ac
#
# If you are working on the trunk (not a release), you will have to
# generate the configure.  If you are using a release, you should have
# a configure provided for you and do not need to run this script.
# These variables are used by autoreconf and will need to be set to your
# proper environment before using this script.

export AUTOCONF=/usr/releng/tools/autoconf/2.59/bin/autoconf 
export AUTOHEADER=/usr/releng/tools/autoconf/2.59/bin/autoheader
export AUTOMAKE=/usr/releng/tools/automake/1.9.4/bin/automake
export ACLOCAL=/usr/releng/tools/automake/1.9.4/bin/aclocal
export LIBTOOLIZE=/usr/releng/tools/libtool/1.5.10/bin/libtoolize
/usr/releng/tools/autoconf/2.59/bin/autoreconf -if
