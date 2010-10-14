#!/bin/sh

set -e -x
[ -x ./configure ] || ./autogen.sh
./configure
make
