AUTOMAKE_OPTIONS = foreign 
SUBDIRS = src icons help 
EXTRA_DIST = COPYING-DOCS\
	README.PERFORMANCE\
	terminatorX.spec.in\
	gnome-doc-utils.make\
	terminatorX.desktop\
	autogen.sh\
	terminatorX.mime

DISTCLEANFILES = gnome-doc-utils.make

# Destkop Entry
desktopdir = $(datadir)/applications
desktop_DATA = terminatorX.desktop

mimedir = $(datadir)/mime-info
mime_DATA = terminatorX.mime terminatorX.keys

man_MANS = terminatorX.1
