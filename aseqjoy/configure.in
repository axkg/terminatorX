dnl Process this file with autoconf to produce a configure script.
AC_INIT(aseqjoy.c)
AM_INIT_AUTOMAKE(aseqjoy, 0.0.1)
AM_CONFIG_HEADER(config.h)

dnl Checks for programs.
AC_PROG_AWK
AC_PROG_CC

dnl Check for headers
AC_CHECK_HEADERS(fcntl.h limits.h sys/ioctl.h alsa/asoundlib.h linux/joystick.h unistd.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

dnl Check for ALSA
AC_CHECK_LIB(asound, snd_seq_event_output_direct,alsalib=yes,alsalib=no)
AC_CHECK_HEADERS(alsa/asoundlib.h,alsaheader=yes,alsaheader=no)

if test "$alsalib" = "yes"; then
	if test "$alsaheader" = "yes"; then
		LIBS="$LIBS -lasound"
	else 
	    AC_MSG_ERROR([** Coulnd't find ALSA header file sys/asoundlib.h **])
    fi
else
    AC_MSG_ERROR([** Coulnd'f find ALSA library libasound. **])
fi

AC_OUTPUT(Makefile aseqjoy.1)
