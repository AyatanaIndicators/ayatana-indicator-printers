printersmenulibdir = $(INDICATORDIR)
printersmenulib_LTLIBRARIES = libprintersmenu.la
libprintersmenu_la_SOURCES = \
	indicator-printers.c \
	indicator-printers.h \
	indicator-menu-item.c \
	indicator-menu-item.h \
	dbus-names.h

libprintersmenu_la_CPPFLAGS = $(APPLET_CFLAGS)
libprintersmenu_la_CFLAGS = $(COVERAGE_CFLAGS)
libprintersmenu_la_LIBADD = $(APPLET_LIBS) -lm
libprintersmenu_la_LDFLAGS = \
	$(COVERAGE_LDFLAGS) \
	-module -avoid-version


cups_notifier_sources = \
	cups-notifier.c \
	cups-notifier.h

$(cups_notifier_sources): org.cups.cupsd.Notifier.xml
	gdbus-codegen \
	    --interface-prefix org.cups.cupsd \
	    --c-namespace Cups \
	    --generate-c-code cups-notifier \
	    $^


pkglibexec_PROGRAMS = ayatana-indicator-printers-service
ayatana_indicator_printers_service_SOURCES = \
	indicator-printers-service.c \
	indicator-printers-menu.c \
	indicator-printers-menu.h \
	indicator-printer-state-notifier.c \
	indicator-printer-state-notifier.h \
	spawn-printer-settings.c \
	spawn-printer-settings.h \
	dbus-names.h

nodist_ayatana_indicator_printers_service_SOURCES = $(cups_notifier_sources)

ayatana_indicator_printers_service_CPPFLAGS = $(SERVICE_CFLAGS)
ayatana_indicator_printers_service_CFLAGS = $(COVERAGE_CFLAGS)
ayatana_indicator_printers_service_LDADD = $(SERVICE_LIBS)
ayatana_indicator_printers_service_LDFLAGS = $(COVERAGE_LDFLAGS)

BUILT_SOURCES = $(cups_notifier_sources)
CLEANFILES= $(BUILT_SOURCES)
EXTRA_DIST = org.cups.cupsd.Notifier.xml

DISTCLEANFILES = \
	Makefile.in