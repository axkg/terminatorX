dnl Process this file with autoconf to produce a configure script.
AC_INIT([terminatorX], [4.0.1], [https://terminatorX.org/bugs/], [terminatorX], [https://terminatorX.org])

AC_CONFIG_SRCDIR([src/tX_global.h])
AM_INIT_AUTOMAKE
AM_CONFIG_HEADER(config.h)

AC_ARG_ENABLE(sox,		[  --enable-sox            use sox as input converter. (default=auto) ])
AC_ARG_ENABLE(mpg123,		[  --enable-mpg123         use mpg123 as input converter. (default=auto) ])
AC_ARG_ENABLE(ogg123,		[  --enable-ogg123         use ogg123 as input converter. (default=auto) ])
AC_ARG_ENABLE(scheduler,	[  --enable-scheduler      enable rt-scheduling (default=auto) ])
AC_ARG_ENABLE(debug,		[  --enable-debug          enable debug output. (default=no) ])
dnl AC_ARG_ENABLE(benchmark	[  --enable-benchmark      creates a non-functional test version (default=no) ])
AC_ARG_ENABLE(wav,		[  --enable-wav            enables the built-in wav-loader (default=yes) ])
AC_ARG_ENABLE(alsa,		[  --enable-alsa           use ALSA for sound output (default=auto) ])
AC_ARG_ENABLE(jack,		[  --enable-jack           use JACK for sound output (default=auto) ])
AC_ARG_ENABLE(oss,		[  --enable-oss            use OSS for sound output (default=auto) ])
AC_ARG_ENABLE(pulse,		[  --enable-pulse          use PulseAudio for sound output (default=auto) ])
AC_ARG_ENABLE(x11,    [  --enable-x11            include X11 specific code (default=auto) ])
AC_ARG_ENABLE(libxml2,	    	[  --disable-libxml2       use libxml even if verion 2 detected (default=auto) ])
AC_ARG_ENABLE(mad,		[  --disable-mad           disable mad for mp3 support (default=auto) ])
AC_ARG_ENABLE(vorbis,		[  --disable-vorbis        disable libvorbis support (default=auto) ])
AC_ARG_ENABLE(audiofile,	[  --disable-audiofile     disable audiofile support (default=auto) ])
AC_ARG_ENABLE(startup,		[  --disable-startup       disable startup-notification (default=auto) ])
AC_ARG_ENABLE(legacy,		[  --enable-legacy         enable support for old terminatorX files (default=no) ])
AC_ARG_ENABLE(alsamidi,		[  --disable-alsamidi      disable support  ALSA MIDI in (default=auto) ])
AC_ARG_ENABLE(lrdf,		[  --disable-lrdf          disable support for liblrdf (default=auto) ])
AC_ARG_ENABLE(capabilities,	[  --enable-capabilities   to use rt-scheduling (default=auto) ])
AC_ARG_ENABLE(deprecated,	[  --disable-deprecated    compile without deprecated G*-headers (default=no) ])
AC_ARG_ENABLE(suidroot,   [  --enable-suidroot      install terminatorX setuid root (default=no) ])
AC_ARG_WITH(docdir,		[  --with-docdir=/some/dir the final location the docs will be installed to. ])

dnl Checks for programs.
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_RANLIB

OPTION_OSS="no"
OPTION_ALSA="no"
OPTION_JACK="no"
OPTION_PULSE="no"
OPTION_X11="no"
OPTION_SOX="no"
OPTION_MPG123="no"
OPTION_MAD="no"
OPTION_OGG123="no"
OPTION_VORBIS="no"
OPTION_AUDIOFILE="no"
OPTION_WAV="no"
OPTION_SCHEDULER="no"
OPTION_ALSAMIDI="no"
OPTION_LEGACY="no"
OPTION_LRDF="no"
OPTION_CAP="no"
OPTION_SUIDROOT="no"

dnl Checks for libraries.
AC_CHECK_LIB(m, floor,, AC_MSG_ERROR([** math-lib not installed or broken **]))

dnl Checking whether -pthreads, -lpthreads or simply nothing

tX_threads=no

dnl Some machines don't require any switches for pthread support...

AC_MSG_CHECKING([whether pthreads works without switches])
	no_threads_flags=no
	AC_LANG_SAVE
	AC_LANG_C
	AC_TRY_LINK([#include <pthread.h>], [ pthread_t thread; pthread_create(&thread, NULL, NULL, NULL);],
		no_threads_flags=yes,
		no_threads_flags=no
	)
	AC_LANG_RESTORE
AC_MSG_RESULT([$no_threads_flags])

if test "$no_threads_flags" = "yes"; then
	tX_threads=auto
fi

dnl Some machines prefer the -lpthread...

if test "$tX_threads" = "no"; then
	pthread_lib=no
	AC_CHECK_LIB(pthread, pthread_detach, pthread_lib=yes, pthread_lib=no)
	
	if test "$pthread_lib" = "yes"; then
		tX_threads=lpthread
		LIBS="$LIBS -lpthread"
	fi
fi

dnl Some machines prefer a -pthread...

if test "$tX_threads" = "no"; then
	AC_MSG_CHECKING([whether -pthread is required for pthreads])
		use_gcc_threads=no
		ac_save_CFLAGS=$CFLAGS
		CFLAGS="$CFLAGS -pthread"
		AC_LANG_SAVE
		AC_LANG_C
		AC_TRY_LINK([#include <pthread.h>], [ pthread_t thread; pthread_create(&thread, NULL, NULL, NULL);],
			use_gcc_threads=yes,
			use_gcc_threads=no
		)
		AC_LANG_RESTORE
		CFLAGS=$ac_save_CFLAGS
	AC_MSG_RESULT([$use_gcc_threads])
	
	if test "$use_gcc_threads" = "yes"; then
		tX_threads=gcc
		CFLAGS="$CFLAGS -pthread"
	fi
fi

dnl If we still haven't found pthreads, we panic...

if test "$tX_threads" = "no"; then
	AC_MSG_ERROR([** failed to detect pthreads implementation **])
fi


AC_SEARCH_LIBS([dlclose], [dl])

dnl Check for GTK

AM_PATH_GTK_3_0(, [
	GTK3_CFLAGS="$GTK_CFLAGS"
	GTK3_LIBS="$GTK_LIBS"
	gtk3="yes"
	GTK3_VERSION="`$PKG_CONFIG --modversion gtk+-3.0`"], [ gtk3="yes" ]) 

if test "$gtk3" = "yes"; then
	AC_DEFINE_UNQUOTED([USE_STARTUP_NOTIFICATION], 1, [Use gtk+ statup notification.])
else
	AC_MSG_ERROR([** couldn't find gtk+ > Version 3. Please upgrade (http://www.gtk.org). **])
fi

AC_PATH_PROG([GLIB_COMPILE_RESOURCES],[glib-compile-resources])
AS_IF([test "$GLIB_COMPILE_RESOURCES" == ""],[
       	AC_MSG_ERROR([glib-compile-resources not found])
       	])

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(fcntl.h limits.h malloc.h unistd.h)

dnl Disable LADSPA caching..
unset ac_cv_header_ladspa_h

have_ladspa=no
AC_CHECK_HEADERS(ladspa.h, have_ladspa=yes, have_ladspa=no)

dnl Disable LADSPA caching..
unset ac_cv_header_ladspa_h

dnl Some people install LADSPA in /usr/local

if test "$have_ladspa" = "no"; then
	ac_save_CFLAGS=$CFLAGS
	CFLAGS="$CFLAGS -I/usr/local/include"
	AC_CHECK_HEADERS(ladspa.h, have_ladspa=yes, have_ladspa=no)
	CFLAGS=$ac_save_CFLAGS
	
	if test "$have_ladspa" = "yes"; then
		CFLAGS="$CFLAGS -I/usr/local/include"
	fi
fi

if test "$have_ladspa" = "no"; then
	AC_MSG_ERROR([** couldn't find ladspa.h - Install the LADSPA SDK from http://www.ladspa.org first. **])
fi

ac_save_CFLAGS=$CFLAGS
CFLAGS="$CFLAGS -finput-charset=utf-8"
AC_TRY_COMPILE([#include <stdio.h>],, have_utf8=yes, have_utf8=no)
CFLAGS=$ac_save_CFLAGS

if test "$have_utf8" = "yes"; then
	CFLAGS="$CFLAGS -finput-charset=utf-8"
fi

AC_CHECK_HEADERS(zlib.h,, AC_MSG_ERROR([** couldn't find zlib.h **]))
AC_CHECK_LIB(z,gzprintf, LIBS="$LIBS -lz", AC_MSG_ERROR([** couldn't find libz library **]))

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_BIGENDIAN
AC_TYPE_SIZE_T

dnl Checks for library functions.
AC_PROG_GCC_TRADITIONAL

dnl Eval args
if test "$enable_closedev" = "no"; then
	AC_DEFINE_UNQUOTED([KEEP_DEV_OPEN], 1, [Define if you want keep device open])
fi


if test "$enable_sox" != "no"; then
 	AC_CHECK_PROG(SOX_PROG, sox, yes)
	if test "$SOX_PROG" = "yes"; then
	 	AC_DEFINE_UNQUOTED([USE_SOX_INPUT], 1, [Define if you want to use sox as input converter])
		OPTION_SOX="yes"
	fi
fi

if test "$enable_mad" != "no"; then
	AC_CHECK_HEADERS(mad.h,madheader=yes,madheader=no)
	if test "$madheader" = "yes"; then
		AC_CHECK_LIB(mad,mad_decoder_init,madlib=yes,madlib=no)
		if test "$madlib" = "yes"; then
			LIBS="$LIBS -lmad"
			AC_DEFINE_UNQUOTED([USE_MAD_INPUT], 1, [Define to use MAD for mp3 support])
			OPTION_MAD="yes"
		fi
	fi
fi

if test "$enable_vorbis" != "no"; then
	AC_CHECK_HEADERS(vorbis/codec.h vorbis/vorbisfile.h,vorbis_headers=yes,vorbis_headers="no")
	if test "$vorbis_headers" = "yes"; then
		AC_CHECK_LIB(vorbis,vorbis_info_init,vorbis_libs=yes,vorbis_libs=no)
		if test "$vorbis_libs" = "yes"; then
			PREV_LIBS=LIBS
			LIBS="$LIBS -lvorbis"
			AC_CHECK_LIB(vorbisfile,ov_open_callbacks,vorbisfile_libs=yes,vorbisfile_libs=no)
			if test "$vorbisfile_libs" = "yes"; then
				LIBS="$LIBS -lvorbisfile"
				AC_DEFINE_UNQUOTED([USE_VORBIS_INPUT], 1, [Define to use libvorbis for ogg support])
				OPTION_VORBIS="yes"
			else
				LIBS=PREV_LIBS
			fi
		fi
	fi
fi

if test "$enable_audiofile" != "no"; then
	AC_CHECK_PROG(af_config,audiofile-config,yes,no)
	if test "$af_config" = "yes"; then
		af_cflags=`audiofile-config --cflags`
		if test "$af_cflags" != ""; then
			CFLAGS="$CFLAGS $af_cflags"
		fi
		af_libs=`audiofile-config --libs`
		LIBS="$LIBS $af_libs"
		AC_DEFINE_UNQUOTED([USE_AUDIOFILE_INPUT], 1, [Define to compile libaudiofile support])
		OPTION_AUDIOFILE="yes"
	else
		AC_CHECK_HEADERS(audiofile.h,audiofile_headers=yes,audiofile_headers=no)
		if test "$audiofile_headers" = "yes"; then
			AC_CHECK_LIB(audiofile,afOpenFile,audiofile_libs=yes,audiofile_libs=no)
			if test "$audiofile_libs" = "yes"; then
				LIBS="$LIBS -laudiofile"
				AC_DEFINE_UNQUOTED([USE_AUDIOFILE_INPUT], 1, [Define to compile libaudiofile support])
				OPTION_AUDIOFILE="yes"
			fi
		fi
	fi
fi

if test "$enable_mpg123" != "no"; then
 	AC_CHECK_PROG(MPG123_PROG, mpg123, yes)
	if test "$MPG123_PROG" = "yes"; then
		if test "$SOX_PROG" = "yes"; then	
		 	AC_DEFINE_UNQUOTED([USE_MPG123_INPUT], 1, [Define mp3-input through mpg123 - requires sox as well])
			OPTION_MPG123="yes"
		else
		 	AC_CHECK_PROG(SOX_PROG, sox, yes)
			if test "$SOX_PROG" = "yes"; then	
			 	AC_DEFINE_UNQUOTED([USE_MPG123_INPUT], 1, [Define mp3-input through mpg123 - requires sox as well])
				OPTION_MPG123="yes"
			else
				AC_MSG_RESULT([** mpg123 support disabled: couldn't find sox! **])
			fi
		fi
	fi
fi

if test "$enable_ogg123" != "no"; then
	AC_CHECK_PROG(OGG123_PROG, ogg123, yes)
	if test "$OGG123_PROG" = "yes"; then
		AC_DEFINE_UNQUOTED([USE_OGG123_INPUT], 1, [Define if you want UGLY ogg-input "support" - also requires sox])
		OPTION_OGG123="yes"
	fi
fi

if test "$enable_movqfix" = "no"; then
	AC_DEFINE_UNQUOTED([OVERRIDE_MOVQ_AUTODETECT], 1, [Do nott use movqfix])
fi

using_alsa=no
using_oss=no
using_jack=no
using_pulse=no

if test "$enable_alsa" != "no"; then
    AC_CHECK_LIB(asound, snd_pcm_writei,alsalib=yes,alsalib=no)
    AC_CHECK_HEADERS(alsa/asoundlib.h,alsaheader=yes,alsaheader=no)

    if test "$alsalib" = "yes"; then
    	    if test "$alsaheader" = "yes"; then
    		    AC_DEFINE_UNQUOTED([USE_ALSA], 1, [ Define to enable ALSA audio backend. ])
		    LIBS="$LIBS -lasound"
    		    using_alsa=yes
		    OPTION_ALSA=yes
    	    else 
    		    if test "$enable_alsa" = "yes"; then
    			    AC_MSG_ERROR([** Could not find ALSA header file sys/asoundlib.h **])
    		    fi
    	    fi
    else
    	    if test "$enable_alsa" = "yes"; then
    		    AC_MSG_ERROR([** Could not find ALSA library libasound. **])
    	    fi
    fi
fi

if test "$enable_jack" != "no"; then
    AC_CHECK_LIB(jack,jack_activate,jacklib=yes,jacklib=no)
    AC_CHECK_HEADERS(jack/jack.h,jackheader=yes,jackheader=no)

    if test "$jacklib" = "yes"; then
    	    if test "$jackheader" = "yes"; then
    		    AC_DEFINE_UNQUOTED([USE_JACK], 1, [ Define to enable JACK audio backend.])
		    LIBS="$LIBS -ljack"
    		    using_jack=yes
		    OPTION_JACK=yes
    	    else 
    		    if test "$enable_jack" = "yes"; then
    			    AC_MSG_ERROR([** Could not find JACK header file jack/jack.h **])
    		    fi
    	    fi
    else
    	    if test "$enable_jack" = "yes"; then
    		    AC_MSG_ERROR([** Could not find JACK library libjack. **])
    	    fi
    fi
fi

if test "$enable_pulse" != "no"; then
    AC_CHECK_LIB(pulse-simple,pa_simple_new,pulselib=yes,pulselib=no)
    AC_CHECK_HEADERS(pulse/pulseaudio.h,pulseheader=yes,pulseheader=no)

    if test "$pulselib" = "yes"; then
    	    if test "$pulseheader" = "yes"; then
    		    AC_DEFINE_UNQUOTED([USE_PULSE], 1, [ Define to enable PULSE audio backend.])
		    LIBS="$LIBS -lpulse-simple -lpulse"
    		    using_pulse=yes
		    OPTION_PULSE=yes
    	    else 
    		    if test "$enable_pulse" = "yes"; then
    			    AC_MSG_ERROR([** Could not find PULSE header file pulse/pulseaudio.h **])
    		    fi
    	    fi
    else
    	    if test "$enable_pulse" = "yes"; then
    		    AC_MSG_ERROR([** Could not find PULSE library libpulse. **])
    	    fi
    fi

fi


if test "$enable_oss" != "no"; then
	AC_CHECK_HEADERS(sys/ioctl.h sys/soundcard.h,oss=yes,oss=no)
	
	if test "$oss" = "yes"; then
		AC_DEFINE_UNQUOTED([USE_OSS], 1, [Use OSS])
		OPTION_OSS=yes
		using_oss=yes;
	else
		if test "$enable_oss" = "yes"; then
			AC_MSG_ERROR([** Could not find OSS header files. ***])
		fi
	fi
fi

if test "$enable_alsamidi" != "no"; then
	if test "$using_alsa" = "yes"; then
		AC_DEFINE_UNQUOTED([USE_ALSA_MIDI_IN], 1, [Define for ALSA MIDI in support])
		OPTION_ALSAMIDI="yes"				
	else
		AC_CHECK_LIB(asound, snd_seq_open,alsalib=yes,alsalib=no)
		AC_CHECK_HEADERS(alsa/asoundlib.h,alsaheader=yes,alsaheader=no)

		if test "$alsalib" = "yes"; then
    			if test "$alsaheader" = "yes"; then
    				AC_DEFINE_UNQUOTED([USE_ALSA_MIDI_IN], 1, [Define for ALSA MIDI in support])
				LIBS="$LIBS -lasound"
				OPTION_ALSAMIDI="yes"
    			else 
    				if test "$enable_alsamidi" = "yes"; then
    					AC_MSG_ERROR([** Coulnd't find ALSA header file alsa/asoundlib.h **])
    				fi
    			fi
		else
    			if test "$enable_alsamidin" = "yes"; then
    				AC_MSG_ERROR([** Couldn't find ALSA library libasound. **])
    			fi
		fi
	fi
fi

if test "$using_alsa" != "yes"; then
	if test "$using_oss" != "yes"; then
		if test "$using_jack" != "yes"; then
			if test "$using_pulse" != "yes"; then
				AC_MSG_ERROR([** Found neither OSS, ALSA, JACK nor PULSE - no output device! **])
			fi
		fi
	fi
fi	

if test "$enable_x11" != "no"; then
		AC_CHECK_LIB(X11, XOpenDisplay, x11lib=yes, x11lib=no)
		AC_CHECK_HEADERS(X11/Xlib.h, x11header=yes, x11header=no)

    if test "$x11lib" = "yes"; then
    	    if test "$x11header" = "yes"; then
    		    AC_DEFINE_UNQUOTED([USE_X11], 1, [ Define to enable X11 specific code (e.g. fullscreen mode ).])
						LIBS="$LIBS -lX11"
    		    using_x11=yes
						OPTION_X11=yes
    	    else 
    		    if test "$enable_x11" = "yes"; then
    			    AC_MSG_ERROR([** X11 headers not installed or broken **])
    		    fi
    	    fi
    else
    	    if test "$enable_x11" = "yes"; then
    		    AC_MSG_ERROR([** X11 libs not installed or broken **])
    	    fi
    fi
fi
	
if test "$enable_wav" != "no";
then
	AC_DEFINE_UNQUOTED([USE_BUILTIN_WAV], 1, [Define to enable the built-in wav loading routines])
	OPTION_WAV="yes"
fi

if test "$enable_3dnow" = "yes"; then
 	AC_DEFINE_UNQUOTED([USE_3DNOW], 1, [Do not define this])
fi 

if test "$enable_scheduler" != "no"; then
	OPTION_SCHEDULER="yes";
	AC_DEFINE_UNQUOTED([USE_SCHEDULER], 1, [Define this to compile with rt scheduling support])
fi

if test "$enable_debug" = yes; then
	AC_DEFINE_UNQUOTED([ENABLE_DEBUG_OUTPUT], 1, [Define this to enable debug output.])
fi

if test "$enable_benchmark" = yes; then
	AC_DEFINE_UNQUOTED([CREATE_BENCHMARK], 1, [Define this to build an performance benchmark - WARNING: You cannot use the resulting binary for normal operation])
fi

if test "$enable_capabilities" != "no"; then
	AC_CHECK_HEADERS(sys/capability.h,capheader=yes,capheader=no)
	AC_CHECK_HEADERS(sys/prctl.h,prctlheader=yes,prctlheader=no)
	
	if test "$capheader" = "yes"; then
		if test "$prctlheader" = "yes"; then
			AC_CHECK_LIB(cap,cap_get_proc,caplib=yes,caplib=no)
			if test "$caplib" = "yes"; then
				LIBS="$LIBS -lcap"
				AC_DEFINE_UNQUOTED([USE_CAPABILITIES], 1, [Define to use capabilities])
				OPTION_CAP="yes"
			fi
		fi
	fi
fi

if test "$enable_capabilities" = "yes"; then
	if test "$OPTION_CAP" != "yes"; then
		AC_MSG_ERROR([** This system doesn't support capabilities. **])
	fi
fi

if test "$enable_deprecated" = "no"; then
	DEPRECATED_FLAGS="-DGTK_DISABLE_DEPRECATED -DGDK_DISABLE_DEPRECATED -DG_DISABLE_DEPRECATED"
else 
    DEPRECATED_FLAGS=""
fi

AC_SUBST(DEPRECATED_FLAGS)

if test "$enable_suidroot" = "yes"; then
	OPTION_SUIDROOT="yes"
else
	OPTION_SUIDROOT="no"
fi

AM_CONDITIONAL([SUIDROOT], [ test x$OPTION_SUIDROOT = xyes])

AC_MSG_CHECKING(for necessary scratching skillz)
AC_MSG_RESULT(yes)

AC_DEFINE_UNQUOTED([USE_DIAL], 1, [ Define this. ])

dnl if test "$enable_bigdial" = yes; then
dnl 	AC_DEFINE(USE_DIAL)
dnl 	AC_DEFINE(USE_BIG_BUTTONS)
dnl fi

if test "$enable_legacy" = yes; then
	AC_DEFINE_UNQUOTED([ENABLE_TX_LEGACY], 1, [Define this to enable support for old tX set files])
	OPTION_LEGACY="yes"
fi

dnl Detecting libxml. Based on dia's test.
found_libxml=false

if test "$enable_libxml2" != "no"; then
	if ! $found_libxml; then
	  AC_CHECK_PROG(XML2_CONFIG, xml2-config, xml2-config)
	  if test "x$XML2_CONFIG" != x ; then
	    AC_MSG_CHECKING(for libxml >= 2.3.9)
	    vers=`$XML2_CONFIG --version | sed -e "s/libxml //" | awk 'BEGIN {FS=".";} { printf "%d", ($1 * 1000 + $2) * 1000 + $3;}'`

	    if test "$vers" -ge 2003009; then
	      AC_MSG_RESULT(yes)
	      found_libxml=true
	      XML_LIBS="`$XML2_CONFIG --libs`"
	      XML_CFLAGS="`$XML2_CONFIG --cflags`"
	    else
	      AC_MSG_RESULT(no)
	    fi
	  fi
	fi
fi	

if ! $found_libxml 
then
	AC_CHECK_PROG(XML_CONFIG, xml-config, xml-config)
	if test "x$XML_CONFIG" != x ; then
	  AC_MSG_CHECKING(for libxml >= 1.8.14)
	  vers=`$XML_CONFIG --version | sed -e "s/libxml //" | awk 'BEGIN {FS=".";} { printf "%d", ($1 * 1000 + $2) * 1000 + $3;}'`

	  dnl 1.18.14 is required for xmlUseNewParser()    
	  if test "$vers" -ge 1008014; then
	    AC_MSG_RESULT(yes)
	    found_libxml=true
	    XML_LIBS="`$XML_CONFIG --libs`"
	    XML_CFLAGS="`$XML_CONFIG --cflags`"
	  else
	    AC_MSG_RESULT(no)
	  fi
	fi
fi

if ! $found_libxml; then
    AC_MSG_ERROR([** libxml not found. Get libxml(2) at http://www.libxml.org **])
fi

AC_SUBST(XML_LIBS)
AC_SUBST(XML_CFLAGS)

if test "$enable_lrdf" != "no"; then
    AC_MSG_CHECKING(for liblrdf >= 0.2.4)
	LRDF_VERS=`$PKG_CONFIG --modversion lrdf | sed -e "s/libxml //" | awk 'BEGIN {FS=".";} { printf "%d", ($1 * 1000 + $2) * 1000 + $3;}'`
	if test "$LRDF_VERS" -ge 2004; then
		AC_MSG_RESULT(yes)
		OPTION_LRDF=yes

		lrdf_cflags=`$PKG_CONFIG --cflags lrdf`
		if test "$lrdf_cflags" != ""; then
			CFLAGS="$CFLAGS $lrdf_cflags"
		fi
		lrdf_libs=`$PKG_CONFIG --libs lrdf`
		LIBS="$LIBS $lrdf_libs"
		AC_DEFINE_UNQUOTED([USE_LRDF], 1, [Define to compile librdf support])
		OPTION_AUDIOFILE="yes"
	else
		AC_MSG_RESULT(no)
	fi

fi

dnl User Manual
GNOME_DOC_INIT

xml_man="\\\"${datadir}/gnome/help/terminatorX-manual/C/terminatorX-manual.xml\\\""


CFLAGS="$XML_CFLAGS $CFLAGS -DXML_MANUAL=$xml_man"
LIBS="$XML_LIBS $LIBS"

AC_OUTPUT(Makefile 
	src/Makefile 
	icons/Makefile 
	src/icons/Makefile 
	help/Makefile
	terminatorX.keys 
	terminatorX.spec 
	terminatorX.1)

option_info() {
	echo "$option support: $option_val"
	if test "$option_val" != "yes"; then
		echo "	- If you want $option support get $option from"
		echo "	-	$option_url"
		echo "	- and reconfigure terminatorX"
	fi
}

echo
echo configured terminatorX for:
echo ---------------------------
echo

option=sox; option_val=$OPTION_SOX; option_url=http://sox.sourceforge.net/
option_info;

echo "mpg123 support: $OPTION_MPG123"
if test "$OPTION_MPG123" = "no"; then
echo "	- If you want mpg123 support get either mpg321 from"
echo "	-	http://mpg321.sourceforge.net/ or mpg123 from"
echo "	-	http://www.mpg123.de/"
echo "	- and reconfigure terminatorX"
fi


option=oss; option_val=$OPTION_OSS; option_url=http://www.kernel.org
option_info;

option=alsa; option_val=$OPTION_ALSA; option_url=http://www.alsa-project.org
option_info;

option=jack; option_val=$OPTION_JACK; option_url=http://jackit.sourceforge.net
option_info;

option=pulse; option_val=$OPTION_PULSE; option_url=http://www.freedesktop.org/wiki/Software/PulseAudio/
option_info;

option=mad; option_val=$OPTION_MAD; option_url=http://www.mars.org/home/rob/proj/mpeg/
option_info;

option=ogg123; option_val=$OPTION_OGG123; option_url=http://www.xiph.org/ogg/vorbis/
option_info;

option=libvorbis; option_val=$OPTION_VORBIS; option_url=http://www.xiph.org/ogg/vorbis/
option_info;

option=audiofile; option_val=$OPTION_AUDIOFILE; option_url=http://www.68k.org/~michael/audiofile/
option_info;

option="ALSA (MIDI in)"; option_val=$OPTION_ALSAMIDI; option_url=http://www.alsa-project.org
option_info;

option=lrdf; option_val=$OPTION_LRDF; option_url=http://plugin.org.uk/releases/lrdf/
option_info;

echo "x11 support: $OPTION_X11"
echo "legacy files support: $OPTION_LEGACY"
echo "builtin-wav support: $OPTION_WAV"
echo "enhanced scheduling support: $OPTION_SCHEDULER"
echo "capabilities support: $OPTION_CAP"
echo "install setuid-root: $OPTION_SUIDROOT"
echo
echo You can now run \'make\' to compile terminatorX

if test "$OPTION_SUIDROOT" = "yes"; then
    echo As setuid-root is enabled, \'make install\' must be run with root privileges.
fi
 
