#!/bin/sh

echo Creating files required for building aseqjoy...
aclocal
autoheader
automake --add-missing
autoconf
echo Now run \"./configure\"
