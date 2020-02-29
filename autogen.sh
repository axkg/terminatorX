#!/bin/sh

echo Creating files required for building terminatorX...
aclocal -Wno-portability
autoheader
automake --add-missing -Wno-portability
autoconf 
echo Now run \"./configure\"
