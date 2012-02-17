
dbus_servicesdir = $(DBUSSERVICEDIR)
dbus_services_DATA = indicator-printers.service

%.service: %.service.in
	sed -e "s|\@libexecdir\@|$(libexecdir)|" $< > $@

EXTRA_DIST = indicator-printers.service.in

CLEANFILES = $(dbus_services_DATA)

