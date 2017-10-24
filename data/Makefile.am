BUILT_SOURCES=
CLEANFILES=
EXTRA_DIST=

SUBDIRS = upstart

#
# the systemd unit file
#

#if defined(HAVE_SYSTEMD)
systemddir = $(SYSTEMD_USERDIR)
systemd_DATA = ayatana-indicator-printers.service
systemd_in = $(systemd_DATA:.service=.service.in)
$(systemd_DATA): $(systemd_in)
	$(AM_V_GEN) $(SED) -e "s|\@pkglibexecdir\@|$(pkglibexecdir)|" $< > $@
BUILT_SOURCES += $(systemd_DATA)
EXTRA_DIST += $(systemd_in)
CLEANFILES += $(systemd_DATA)

#
# the systemd upstart override
#

upstart_overridedir = $(datadir)/upstart/systemd-session/upstart
upstart_override_DATA = ayatana-indicator-printers.override
EXTRA_DIST += $(upstart_override_DATA)
#endif

#
# the upstart job file
#

upstart_jobsdir = $(datadir)/upstart/sessions
upstart_jobs_DATA = ayatana-indicator-printers.conf
upstart_jobs_in = $(upstart_jobs_DATA:.conf=.conf.in)
$(upstart_jobs_DATA): $(upstart_jobs_in)
	$(AM_V_GEN) $(SED) -e "s|\@pkglibexecdir\@|$(pkglibexecdir)|" $< > $@
BUILT_SOURCES += $(upstart_jobs_DATA)
CLEANFILES += $(upstart_jobs_DATA)
EXTRA_DIST += $(upstart_jobs_in)

#
# the xdg autostart job file
#

xdg_autostartdir = /etc/xdg/autostart
xdg_autostart_DATA = indicator-printers.desktop
xdg_autostart_in = $(xdg_autostart_DATA:.desktop=.desktop.in)
$(xdg_autostart_DATA): $(xdg_autostart_in)
	$(AM_V_GEN) $(SED) -e "s|\@pkglibexecdir\@|$(pkglibexecdir)|" $< > $@
BUILT_SOURCES += $(xdg_autostart_DATA)
CLEANFILES += $(xdg_autostart_DATA)
EXTRA_DIST += $(xdg_autostart_in)

DISTCLEANFILES = \
	Makefile.in

