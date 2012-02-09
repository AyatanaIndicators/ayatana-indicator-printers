
printersmenulibdir = $(INDICATORDIR)
printersmenulib_LTLIBRARIES = libprintersmenu.la
libprintersmenu_la_SOURCES = \
	indicator-printers.c \
	indicator-printers.h \
	indicator-menu-item.c \
	indicator-menu-item.h

libprintersmenu_la_CPPFLAGS = $(APPLET_CFLAGS)
libprintersmenu_la_LIBADD = $(APPLET_LIBS)
libprintersmenu_la_LDFLAGS = -module -avoid-version


cups_notifier_sources = \
	cups-notifier.c \
	cups-notifier.h

$(cups_notifier_sources): org.cups.cupsd.Notifier.xml
	gdbus-codegen \
	    --interface-prefix org.cups.cupsd \
	    --c-namespace Cups \
	    --generate-c-code cups-notifier \
	    $^


libexec_PROGRAMS = indicator-printers-service
indicator_printers_service_SOURCES = \
	indicator-printers-service.c \
	indicator-printers-menu.c \
	indicator-printers-menu.h \
	indicator-printer-state-notifier.c \
	indicator-printer-state-notifier.h \
	$(cups_notifier_sources)

indicator_printers_service_CPPFLAGS = $(SERVICE_CFLAGS)
indicator_printers_service_LDADD = $(SERVICE_LIBS)


BUILT_SOURCES = $(cups_notifier_sources)

