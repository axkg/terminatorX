LIBS=@GTK_LIBS@ -lXi -lXxf86dga -lpthread

CFLAGS=@CFLAGS@ @GTK_CFLAGS@ -D_REENTRANT
CXXFLAGS=@CFLAGS@ @GTK_CFLAGS@ -D_REENTRANT

license.c: $(top_srcdir)/COPYING
	@AWK@ -f $(srcdir)/licmak.awk < $(top_srcdir)/COPYING > $(srcdir)/license.c

BUILT_SOURCES = license.c

bin_PROGRAMS = terminatorX

terminatorX_DEPENDENCIES = license.c

terminatorX_SOURCES =	endian.c tX_dialog.cc tX_widget.c wav_write.c \
			tX_engine.cc tX_logo.c tX_mouse.cc tX_mouse.h \
			main.cc tX_global.c tX_wavfunc.cc wav_read.c licmak.awk \
			endian.h tX_engine.h tX_types.h tX_flash.c tX_flash.h \
			tX_global.h tX_wavfunc.h version.h tX_tape.h tX_tape.cc \
			tX_dialog.h tX_mastergui.h tX_widget.h wav_file.h \
			tX_vtt.cc tX_vtt.h tX_vttgui.h tX_vttgui.cc \
			tX_audiodevice.cc tX_audiodevice.h tX_mastergui.cc \
			Makefile.cygwin 3dnow.h mmx.h tX_smlog.c
