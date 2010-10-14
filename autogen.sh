#!/bin/sh
# Run this to generate all the initial makefiles, etc.

set -e -x
aclocal
autoheader
automake -a -c -f --foreign
autoconf
