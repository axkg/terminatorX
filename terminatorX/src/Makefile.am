LIBS=@GTK_LIBS@ -lXi -lXxf86dga -lpthread

CFLAGS=@CFLAGS@ @GTK_CFLAGS@ -D_REENTRANT

license.c: $(top_srcdir)/COPYING
	@AWK@ -f $(srcdir)/licmak.awk < $(top_srcdir)/COPYING > $(srcdir)/license.c

BUILT_SOURCES = license.c

bin_PROGRAMS = terminatorX

terminatorX_DEPENDENCIES = license.c

terminatorX_SOURCES =	endian.c tX_dialog.c tX_gui.c tX_widget.c wav_write.c \
			tX_engine.c tX_logo.c turntable.c \
			main.c tX_global.c tX_wavfunc.c wav_read.c licmak.awk \
			endian.h tX_engine.h tX_types.h turntable.h \
			tX_global.h tX_wavfunc.h version.h \
			tX_dialog.h tX_gui.h tX_widget.h wav_file.h \
			Makefile.cygwin
