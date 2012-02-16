
noinst_PROGRAMS = mock-cups-notifier
DISTCLEANFILES = mock-cups-notifier

cups_notifier_sources = \
	cups-notifier.c \
	cups-notifier.h

$(cups_notifier_sources): $(top_srcdir)/src/org.cups.cupsd.Notifier.xml
	gdbus-codegen \
	    --interface-prefix org.cups.cupsd \
	    --c-namespace Cups \
	    --generate-c-code cups-notifier \
	    $^

mock_cups_notifier_SOURCES = \
	mock-cups-notifier.c \
	$(cups_notifier_sources)

mock_cups_notifier_CPPFLAGS = \
	$(SERVICE_CFLAGS) \
	-I$(top_builddir)/src

mock_cups_notifier_LDADD = $(SERVICE_LIBS)

BUILT_SOURCES = $(cups_notifier_sources)

