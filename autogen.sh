#!/bin/sh

echo Creating files required for building terminatorX...
aclocal
autoheader
automake --add-missing
autoconf
echo Now run \"./configure\"
