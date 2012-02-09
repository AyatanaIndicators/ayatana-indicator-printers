
bin_PROGRAMS = mock-cups-notifier
DISTCLEANFILES = mock-cups-notifier

mock_cups_notifier_SOURCES = \
	mock-cups-notifier.c \
	$(top_builddir)/src/cups-notifier.c \
	$(top_builddir)/src/cups-notifier.h

mock_cups_notifier_CPPFLAGS = \
	$(SERVICE_CFLAGS) \
	-I$(top_builddir)/src

mock_cups_notifier_LDADD = $(SERVICE_LIBS)

