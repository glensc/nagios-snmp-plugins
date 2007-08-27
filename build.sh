#! /bin/sh

touch ChangeLog
aclocal
autoheader
automake --add-missing
autoconf
./configure
make
rm -f ChangeLog
