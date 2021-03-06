terminatorX INSTALL
===================

Copyright (C) 1999-2022 by Alexander König <alex@lisas.de>
https://terminatorX.org

Quickstart
----------

If you want enhanced audio-file support or optimization don't
"quickstart" but read the rest of this file.

If you just cant wait any longer try:
> ./configure
> make install

Step 1: Configure terminatorX
-----------------------------

If you want your compiler to optimize the binary, you need to set your
environment variable CFLAGS to your needs BEFORE you run ./configure.

for example:
if use bash: > export CFLAGS="-O2"
with tcsh:   > setenv CFLAGS "-O2"

Optionally you might want to add additional tuning parameters for
your target platform.

All of the following configure options can be either enabled with
--enable-option or disabled with --disable-option.

The ./configure Options
-----------------------

### File support options

All of these are enabled by default. Nevertheless the configure
script checks for the availability of the helper application
and if it's not found disables support for it. Check the
terminatorX homepage for links to those apps if you don't have
them - or check your distribution first, AFAIK all bring these
tools nowaday.

--enable-mad

This will make terminatorX use the MPEG Audio Decoder library if
it's detected. This allows terminatorX to load mp3 files
significantly faster than with the mpg123 method. Additionally
terminatorX will find out about the sampling rate of an mp3 file
and adjust the playback speed accordingly.

--enable-vorbis

This will make terminatorX use the OGG Vobris libraries to load
OGG files directly. This method has the same advantages over
loading through ogg123 as the "mad" method has over loading
through mpg123.

--enable-audiofile

This enables the use of libaudiofile on loading audio files.
The library supports a wide range of common audio file formats
(eg WAV/AIFF/AU etc) therefor its use highly recommended.

--enable-wav

This enables the builtin wav routines. They load 16Bit/44Khz
MONO RIFF/WAV files only but they do that significantly faster
than using sox. If these routines fail and sox support is
enabled, terminatorX will try to load the file with sox as
a fallback. Disable them only if they don't load your files
correctly.

--enable-sox

This enables sox support. As sox can load nearly any audio file
it makes sense to use it. You have to have sox installed of
course.

--enable-mpg123

This enables mpg123 support. If you want to be able to load mp3
files keep this option enabled. You have to have mpg123
installed of course.

--enable-ogg123

With this option you can turn on/off support for Ogg Vorbis
soundfiles. This requires ogg123 (Version >= 1.0RC2) and
sox to be installed.

--enable-suidroot

When set, this option sets the setuid bits for the root user when
installing the binary, note that this will require root privileges
upon installation.

## Other options

--enable-capabilities

Allows running terminatorX suid-root to gain realtime scheduling
(see README.PERFORMANCE).

--with-docdir

If you intend to package terminatorX this flag will allow
terminatorX to find the XML documentation in order to display
it online.

--disable-libxml2

If you've got libxml V2 installed but you want terminatorX to
use V1 instead, use this to disable libxml V1.

--enable-debug

This will cause terminatorX to display some debug messages on
your console.

Step 2: Build the binary
------------------------

This one's easy:
> make

Step 3: Install the binary
--------------------------

Just as easy:
> make install

Typically you will need root privileges to install in the standard prefix,
if installing as root is not an option you can use the --prefix configure
switch to install to another location.
