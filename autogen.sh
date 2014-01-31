#!/bin/sh

echo Creating files required for building terminatorX...
gnome-doc-prepare --force --automake
aclocal -Wno-portability
autoheader
automake --add-missing -Wno-portability
autoconf 
echo Now run \"./configure\"
