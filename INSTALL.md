# terminatorX INSTALL

Copyright (C) 1999-2024 by Alexander König <alex@lisas.de>
https://terminatorX.org

## Quickstart

If you want enhanced audio-file support or optimization don't
"quickstart" but read the rest of this file.

To build terminatorX with default options run the following commands (using
'build' as the build directory):

```
$ meson setup build
$ meson compile -C build
$ sudo meson install -C build
```

The following sections descripe the installation steps in more detail.

## Step 1: Configure the terminatorX Build

The Meson Build system supports a number of [built-in
options](https://mesonbuild.com/Builtin-options.html). The default build type
for terminatorX is 'debugoptimized'. You may want to change that, e.g. if you
you don't want the binary to carry debug information you could select 'release'
instead:

```
$ meson setup build --buildtype=release
```

In addition to the built-in options, the terminatorX meson build
supports a number of additional [options](meson.options) to customize
the build. These features can be either en- or disabled:

```
$ meson setup build -Dfeature=enabled
```

Where 'feature' can be anyone of the features listed in the following 
section. It is possible to probide the '-D' argument multiple times to
configure multiple features.

## Meson Build Options

### File support options

All of these are enabled by default. Nevertheless the configure
script checks for the availability of the helper application
and if it's not found disables support for it. Check the
terminatorX homepage for links to those apps if you don't have
them - or check your distribution first, AFAIK all bring these
tools nowaday.

#### mad

This will make terminatorX use the MPEG Audio Decoder library if
it's detected. This allows terminatorX to load mp3 files
significantly faster than with the mpg123 method. Additionally
terminatorX will find out about the sampling rate of an mp3 file
and adjust the playback speed accordingly.

#### vorbis

This will make terminatorX use the OGG Vobris libraries to load
OGG files directly. This method has the same advantages over
loading through ogg123 as the "mad" method has over loading
through mpg123.

#### audiofile

This enables the use of libaudiofile on loading audio files.
The library supports a wide range of common audio file formats
(eg WAV/AIFF/AU etc) therefor its use highly recommended.

#### wav

This enables the builtin wav routines. They load 16Bit/44Khz
MONO RIFF/WAV files only but they do that significantly faster
than using sox. If these routines fail and sox support is
enabled, terminatorX will try to load the file with sox as
a fallback. Disable them only if they don't load your files
correctly.

#### sox

This enables sox support. As sox can load nearly any audio file
it makes sense to use it. You have to have sox installed of
course.

#### mpg123

This enables mpg123 support. If you want to be able to load mp3
files keep this option enabled. You have to have mpg123
installed of course.

#### ogg123

With this option you can turn on/off support for Ogg Vorbis
soundfiles. This requires ogg123 (Version >= 1.0RC2) and
sox to be installed.

## Other options

#### suidroot

When set, this option sets the setuid bits for the root user when
installing the binary, note that this will require root privileges
upon installation.

#### capabilities

Allows running terminatorX suid-root to gain realtime scheduling
(see README.PERFORMANCE).

#### debug

This will cause terminatorX to display some debug messages on
your console.

## Step 2: Build the binary

This one's easy:

```
$ meson compile -C build
```

## Step 3: Install the binary

Just as easy:

```
$ sudo meson install -C build
```

The above example uses 'sudo' to gain root privileges, assuming the standard
installation prefix was used (typically '/usr/local'). Other installation
prefixes might not require root privileges, however if you chose the 'suidroot'
option, root will be required in any case.
