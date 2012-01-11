
printersmenulibdir = $(INDICATORDIR)
printersmenulib_LTLIBRARIES = libprintersmenu.la
libprintersmenu_la_SOURCES = \
	indicator-printers.c \
	indicator-printers.h

libprintersmenu_la_CPPFLAGS = $(APPLET_CFLAGS)
libprintersmenu_la_LIBADD = $(APPLET_LIBS)
libprintersmenu_la_LDFLAGS = -module -avoid-version


libexec_PROGRAMS = indicator-printers-service
indicator_printers_service_SOURCES = \
	indicator-printers-service.c \
	indicator-printers-service.h

indicator_printers_service_CPPFLAGS = $(SERVICE_CFLAGS)
indicator_printers_service_LDADD = $(SERVICE_LIBS)

