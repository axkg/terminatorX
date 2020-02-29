LIBS=@GTK_LIBS@ @LIBS@

SUBDIRS = icons

EXTRA_CFLAGS=-D_REENTRANT @DEPRECATED_FLAGS@
AM_CFLAGS=@CFLAGS@ @GTK_CFLAGS@ $(EXTRA_CFLAGS)
AM_CXXFLAGS=@CFLAGS@ @GTK_CFLAGS@  $(EXTRA_CFLAGS)

license.c: $(top_srcdir)/COPYING
	@AWK@ -f $(srcdir)/licmak.awk < $(top_srcdir)/COPYING > $(srcdir)/license.c

tX_dialog.o: license.c

BUILT_SOURCES = license.c
CLEANFILES = license.c

bin_PROGRAMS = terminatorX

terminatorX_DEPENDENCIES = license.c

if SUIDROOT
install-exec-hook:
	chown root $(DESTDIR)/$(bindir)/terminatorX
	chmod u+s $(DESTDIR)/$(bindir)/terminatorX
endif

terminatorX_SOURCES =	tX_endian.c tX_dialog.cc tX_widget.c wav_write.c \
			tX_engine.cc tX_logo.c tX_mouse.cc tX_mouse.h \
			main.cc tX_global.c wav_read.c licmak.awk \
			tX_endian.h tX_engine.h tX_types.h tX_flash.c tX_flash.h \
			tX_global.h version.h tX_tape.h tX_tape.cc \
			tX_dialog.h tX_mastergui.h tX_widget.h wav_file.h \
			tX_vtt.cc tX_vtt.h tX_vttgui.h tX_vttgui.cc \
			tX_audiodevice.cc tX_audiodevice.h tX_mastergui.cc \
			tX_audiofile.cc tX_audiofile.h tX_loaddlg.cc \
			tX_loaddlg.h tX_prelis.cc tX_prelis.h \
			tX_seqpar.h tX_seqpar.cc tX_pbutton.cc tX_pbutton.h \
			tX_event.h tX_event.cc tX_sequencer.h tX_sequencer.cc \
			tX_knobloader.h tX_knobloader.c tX_dial.c tX_dial.h \
			tX_extdial.h tX_extdial.cc tX_panel.h tX_panel.cc \
			tX_ladspa.h tX_ladspa.cc tX_vttfx.h tX_vttfx.cc \
			tX_legacy_vtt.cc tX_legacy_global.c \
			tX_midiin.h tX_midiin.cc tX_ui_interface.cc \
			tX_ui_interface.h tX_ui_callbacks.cc \
			tX_ui_callbacks.h tX_ui_support.cc \
			tX_ui_support.h tX_ladspa_class.h tX_ladspa_class.cc \
			tX_capabilities.h tX_capabilities.cc

terminatorX_LDADD = $(LDADD) -ldl
